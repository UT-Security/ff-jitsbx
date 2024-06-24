/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jitsbx/JitSandboxMemory.h"

#include "mozilla/Array.h"
#include "mozilla/Assertions.h"
#include "mozilla/Atomics.h"
#include "mozilla/TaggedAnonymousMemory.h"

#include <sys/mman.h>
#include <unistd.h>

#include "jitsbx/JitSandbox.h"
#include "js/HeapAPI.h"
#include "threading/LockGuard.h"
#include "threading/Mutex.h"
#include "vm/MutexIDs.h"

namespace js {

namespace jitsbx {
  
static size_t pageSize = 0;

enum class Commit : bool {
  No = false,
  Yes = true,
};

enum class PageAccess : int {
  None = PROT_NONE,
  Read = PROT_READ,
  ReadWrite = PROT_READ | PROT_WRITE,
  Execute = PROT_EXEC,
  ReadExecute = PROT_READ | PROT_EXEC,
  ReadWriteExecute = PROT_READ | PROT_WRITE | PROT_EXEC,
};

/* Returns the offset from the nearest aligned address at or below |region|. */
static inline size_t OffsetFromAligned(void* region, size_t alignment) {
  return uintptr_t(region) % alignment;
}


template <Commit commit, PageAccess prot>
static inline void* MapInternal(void* desired, size_t length) {
  void* region = nullptr;
  int flags = MAP_PRIVATE | MAP_ANON;
  region = MozTaggedAnonymousMmap(desired, length, int(prot), flags, -1, 0,
                                  "js-gc-heap");
  if (region == MAP_FAILED) {
    return nullptr;
  }
  return region;
}

static inline void ProtectMemory(void* region, size_t length, PageAccess prot) {
  MOZ_RELEASE_ASSERT(region && OffsetFromAligned(region, pageSize) == 0);
  MOZ_RELEASE_ASSERT(length > 0 && length % pageSize == 0);

  MOZ_RELEASE_ASSERT(mprotect(region, length, int(prot)) == 0);
}

void ProtectPages(void* region, size_t length) {
  ProtectMemory(region, length, PageAccess::None);
}

void UnprotectPages(void* region, size_t length) {
  ProtectMemory(region, length, PageAccess::ReadWrite);
}

template <size_t NumBits>
class ChunkBitSet {
  using WordType = uint32_t;

  static const size_t BitsPerWord = sizeof(WordType) * 8;

  static_assert((NumBits % BitsPerWord) == 0,
                "NumBits must be a multiple of BitsPerWord");
  static const size_t NumWords = NumBits / BitsPerWord;

  mozilla::Array<WordType, NumWords> words_;

  uint32_t indexToWord(uint32_t index) const {
    MOZ_ASSERT(index < NumBits);
    return index / BitsPerWord;
  }
  WordType indexToBit(uint32_t index) const {
    MOZ_ASSERT(index < NumBits);
    return WordType(1) << (index % BitsPerWord);
  }

 public:
  void init() { mozilla::PodArrayZero(words_); }
  bool contains(size_t index) const {
    uint32_t word = indexToWord(index);
    return words_[word] & indexToBit(index);
  }
  void insert(size_t index) {
    MOZ_ASSERT(!contains(index));
    uint32_t word = indexToWord(index);
    words_[word] |= indexToBit(index);
  }
  void remove(size_t index) {
    MOZ_ASSERT(contains(index));
    uint32_t word = indexToWord(index);
    words_[word] &= ~indexToBit(index);
  }

#ifdef DEBUG
  bool empty() const {
    for (size_t i = 0; i < NumWords; i++) {
      if (words_[i] != 0) {
        return false;
      }
    }
    return true;
  }
#endif
};

class HeapMemory {
  static_assert(JITSBX_HEAP_SIZE % js::gc::ChunkSize == 0,
                "JitSandox heap size must be a multiple of GC Chunk Size");

  static const size_t MaxHeapChunks = JITSBX_HEAP_SIZE / js::gc::ChunkSize;

  uint8_t* base_;

  Mutex lock_ MOZ_UNANNOTATED;

  mozilla::Atomic<size_t, mozilla::ReleaseAcquire> chunksAllocated_;

  size_t cursor_;

  ChunkBitSet<MaxHeapChunks> chunks_;

 public:
  HeapMemory()
      : base_(nullptr),
        lock_(mutexid::ProcessExecutableRegion),
        chunksAllocated_(0),
        cursor_(0),
        chunks_() {}

  [[nodiscard]] bool init() {
    chunks_.init();

    MOZ_RELEASE_ASSERT(!initialized());

    void* p = MapInternal<Commit::No, PageAccess::None>((void*)JITSBX_HEAP_BASE,
                                                        JITSBX_HEAP_SIZE);
    if (!p) {
      return false;
    }

    base_ = static_cast<uint8_t*>(p);

    return true;
  }

  uint8_t* base() const { return base_; }

  bool initialized() const { return base_ != nullptr; }

  void* allocate(size_t bytes, size_t alignment);
  void deallocate(void* addr, size_t bytes);
};

void* HeapMemory::allocate(size_t bytes, size_t alignment) {
  MOZ_ASSERT(initialized());
  MOZ_ASSERT(bytes > 0);
  MOZ_ASSERT(bytes % alignment == 0);
  MOZ_ASSERT(alignment % js::gc::ChunkSize == 0);

  size_t numChunks = bytes / js::gc::ChunkSize;

  // Take the lock and try to allocate.
  void* p = nullptr;
  {
    LockGuard<Mutex> guard(lock_);
    MOZ_ASSERT(chunksAllocated_ <= MaxHeapChunks);

    // Check if we have enough pages available.
    if (chunksAllocated_ + numChunks >= MaxHeapChunks) {
      return nullptr;
    }

    MOZ_ASSERT(bytes <= JITSBX_HEAP_SIZE);

    size_t chunk = cursor_;

    for (size_t i = 0; i < MaxHeapChunks; i++) {
      // Make sure page + numPages - 1 is a valid index.
      if (chunk + numChunks > MaxHeapChunks) {
        chunk = 0;
      }

      bool available = true;
      for (size_t j = 0; j < numChunks; j++) {
        if (chunks_.contains(chunk + j)) {
          available = false;
          break;
        }
      }
      if (!available) {
        chunk++;
        continue;
      }

      // Mark the pages as unavailable.
      for (size_t j = 0; j < numChunks; j++) {
        chunks_.insert(chunk + j);
      }

      chunksAllocated_ += numChunks;
      MOZ_ASSERT(chunksAllocated_ <= MaxHeapChunks);

      // If we allocated a small number of pages, move cursor_ to the
      // next page. We don't do this for larger allocations to avoid
      // skipping a large number of small holes.
      if (numChunks <= 2) {
        cursor_ = chunk + numChunks;
      }

      p = base_ + chunk * js::gc::ChunkSize;
      break;
    }
    if (!p) {
      return nullptr;
    }
  }

  // Commit the pages after releasing the lock.
  UnprotectPages(p, bytes);
  return p;
}

void HeapMemory::deallocate(void* addr, size_t bytes) {
  MOZ_ASSERT(initialized());
  MOZ_ASSERT(addr);
  MOZ_ASSERT((uintptr_t(addr) % gc::ChunkSize) == 0);
  MOZ_ASSERT(bytes > 0);
  MOZ_ASSERT((bytes % gc::ChunkSize) == 0);

  size_t firstChunk = (static_cast<uint8_t*>(addr) - base_) / gc::ChunkSize;
  size_t numChunks = bytes / gc::ChunkSize;

  ProtectPages(addr, bytes);

  LockGuard<Mutex> guard(lock_);
  MOZ_ASSERT(numChunks <= chunksAllocated_);
  chunksAllocated_ -= numChunks;

  for (size_t i = 0; i < numChunks; i++) {
    chunks_.remove(firstChunk + i);
  }

  // Move the cursor back so we can reuse pages instead of fragmenting the
  // whole region.
  if (firstChunk < cursor_) {
    cursor_ = firstChunk;
  }
}

static HeapMemory heapMemory;

void InitHeapMemory() {
  pageSize = size_t(sysconf(_SC_PAGESIZE));

  MOZ_RELEASE_ASSERT(heapMemory.init(),
                     "Failed to initialize JitSandbox heap memory");

  // Initialize gsbase
  //__asm__ __volatile__("wrgsbase %0" : : "r"(JITSBX_HEAP_BASE));
}

void* MapAlignedPages(size_t length, size_t alignment) {
  return heapMemory.allocate(length, alignment);
}

void UnmapPages(void* region, size_t length) {
  heapMemory.deallocate(region, length);
}

};  // namespace jitsbx

};  // namespace js
