/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jitsbx/JitSandboxExecutableMemory.h"

#include "mozilla/TaggedAnonymousMemory.h"
#include "mozilla/XorShift128PlusRNG.h"

#include "jsmath.h"

#include "gc/Memory.h"
#include "jit/FlushICache.h"
#include "threading/LockGuard.h"
#include "threading/Mutex.h"
#include "util/Memory.h"
#include "vm/MutexIDs.h"


using namespace js;
using namespace js::jitsbx;

static void* ReserveJitSandboxExecutableMemory(size_t bytes) {
  // Note that randomAddr is just a hint: if the address is not available
  // mmap will pick a different address.
  void* randomAddr = (void*)(uintptr_t(jitsbx::ExecutableMemoryBase));
  void* p = MozTaggedAnonymousMmap(randomAddr, bytes, PROT_NONE,
                                   MAP_NORESERVE | MAP_FIXED | MAP_PRIVATE | MAP_ANON, -1,
                                   0, "jitsbx-executable-memory");
  if (p == MAP_FAILED) {
    return nullptr;
  }
  return p;
}

static void DeallocateJitSandboxExecutableMemory(void* addr, size_t bytes) {
  mozilla::DebugOnly<int> result = munmap(addr, bytes);
  MOZ_ASSERT(!result || errno == ENOMEM);
}

static unsigned ProtectionSettingToFlags(ProtectionSetting protection) {
#  ifdef MOZ_VALGRIND
  // If we're configured for Valgrind and running on it, use a slacker
  // scheme that doesn't change execute permissions, since doing so causes
  // Valgrind a lot of extra overhead re-JITting code that loses and later
  // regains execute permission.  See bug 1338179.
  if (RUNNING_ON_VALGRIND) {
    switch (protection) {
      case ProtectionSetting::Protected:
        return PROT_NONE;
      case ProtectionSetting::Writable:
        return PROT_READ | PROT_WRITE | PROT_EXEC;
      case ProtectionSetting::Executable:
        return PROT_READ | PROT_EXEC;
    }
    MOZ_CRASH();
  }
  // If we get here, we're configured for Valgrind but not running on
  // it, so use the standard scheme.
#  endif
  switch (protection) {
    case ProtectionSetting::Protected:
      return PROT_NONE;
    case ProtectionSetting::Writable:
      return PROT_READ | PROT_WRITE;
    case ProtectionSetting::Executable:
      return PROT_READ | PROT_EXEC;
  }
  MOZ_CRASH();
}

[[nodiscard]] static bool CommitPages(void* addr, size_t bytes,
                                      ProtectionSetting protection) {
  void* p = MozTaggedAnonymousMmap(
      addr, bytes, ProtectionSettingToFlags(protection),
      MAP_FIXED | MAP_PRIVATE | MAP_ANON, -1, 0, "jitsbx-executable-memory");
  if (p == MAP_FAILED) {
    return false;
  }
  MOZ_RELEASE_ASSERT(p == addr);
  return true;
}

static void DecommitPages(void* addr, size_t bytes) {
  // Use mmap with MAP_FIXED and PROT_NONE. Inspired by jemalloc's
  // pages_decommit.
  void* p = MozTaggedAnonymousMmap(addr, bytes, PROT_NONE,
                                   MAP_FIXED | MAP_PRIVATE | MAP_ANON, -1, 0,
                                   "jitsbx-executable-memory");
  MOZ_RELEASE_ASSERT(addr == p);
}

template <size_t NumBits>
class PageBitSet {
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

class JitSandboxExecutableMemory {
  static_assert(
      (MaxCodeBytesPerProcess % ExecutableCodePageSize) == 0,
      "MaxCodeBytesPerProcess must be a multiple of ExecutableCodePageSize");
  static const size_t MaxCodePages =
      MaxCodeBytesPerProcess / ExecutableCodePageSize;

  // Start of the MaxCodeBytesPerProcess memory block or nullptr if
  // uninitialized. Note that this is NOT guaranteed to be aligned to
  // ExecutableCodePageSize.
  uint8_t* base_;

  // The fields below should only be accessed while we hold the lock.
  Mutex lock_ MOZ_UNANNOTATED;

  // pagesAllocated_ is an Atomic so that bytesAllocated does not have to
  // take the lock.
  mozilla::Atomic<size_t, mozilla::ReleaseAcquire> pagesAllocated_;

  // Page where we should try to allocate next.
  size_t cursor_;

  mozilla::Maybe<mozilla::non_crypto::XorShift128PlusRNG> rng_;
  PageBitSet<MaxCodePages> pages_;

 public:
  JitSandboxExecutableMemory()
      : base_(nullptr),
        lock_(mutexid::ProcessExecutableRegion),
        pagesAllocated_(0),
        cursor_(0),
        rng_(),
        pages_() {}

  [[nodiscard]] bool init() {
    pages_.init();

    MOZ_RELEASE_ASSERT(!initialized());
    MOZ_RELEASE_ASSERT(gc::SystemPageSize() <= ExecutableCodePageSize);

    void* p = ReserveJitSandboxExecutableMemory(MaxCodeBytesPerProcess);
    if (!p) {
      return false;
    }

    base_ = static_cast<uint8_t*>(p);

    mozilla::Array<uint64_t, 2> seed;
    GenerateXorShift128PlusSeed(seed);
    rng_.emplace(seed[0], seed[1]);
    return true;
  }

  uint8_t* base() const { return base_; }

  bool initialized() const { return base_ != nullptr; }

  size_t bytesAllocated() const {
    MOZ_ASSERT(pagesAllocated_ <= MaxCodePages);
    return pagesAllocated_ * ExecutableCodePageSize;
  }

  void release() {
    MOZ_ASSERT(initialized());
    MOZ_ASSERT(pages_.empty());
    MOZ_ASSERT(pagesAllocated_ == 0);
    DeallocateJitSandboxExecutableMemory(base_, MaxCodeBytesPerProcess);
    base_ = nullptr;
    rng_.reset();
    MOZ_ASSERT(!initialized());
  }

  void assertValidAddress(void* p, size_t bytes) const {
    MOZ_RELEASE_ASSERT(p >= base_ &&
                       uintptr_t(p) + bytes <=
                           uintptr_t(base_) + MaxCodeBytesPerProcess);
  }

  bool containsAddress(const void* p) const {
    return p >= base_ &&
           uintptr_t(p) < uintptr_t(base_) + MaxCodeBytesPerProcess;
  }

  void* allocate(size_t bytes, ProtectionSetting protection,
                 MemCheckKind checkKind);
  void deallocate(void* addr, size_t bytes, bool decommit);
};

void* JitSandboxExecutableMemory::allocate(size_t bytes,
                                        ProtectionSetting protection,
                                        MemCheckKind checkKind) {
  MOZ_ASSERT(initialized());
  MOZ_RELEASE_ASSERT(gc::SystemPageSize() <= ExecutableCodePageSize);
  MOZ_ASSERT(bytes > 0);
  MOZ_ASSERT((bytes % ExecutableCodePageSize) == 0);

  size_t numPages = bytes / ExecutableCodePageSize;

  // Take the lock and try to allocate.
  void* p = nullptr;
  {
    LockGuard<Mutex> guard(lock_);
    MOZ_ASSERT(pagesAllocated_ <= MaxCodePages);

    // Check if we have enough pages available.
    if (pagesAllocated_ + numPages >= MaxCodePages) {
      return nullptr;
    }

    MOZ_ASSERT(bytes <= MaxCodeBytesPerProcess);

    // Maybe skip a page to make allocations less predictable.
    size_t page = cursor_ + (rng_.ref().next() % 2);

    for (size_t i = 0; i < MaxCodePages; i++) {
      // Make sure page + numPages - 1 is a valid index.
      if (page + numPages > MaxCodePages) {
        page = 0;
      }

      bool available = true;
      for (size_t j = 0; j < numPages; j++) {
        if (pages_.contains(page + j)) {
          available = false;
          break;
        }
      }
      if (!available) {
        page++;
        continue;
      }

      // Mark the pages as unavailable.
      for (size_t j = 0; j < numPages; j++) {
        pages_.insert(page + j);
      }

      pagesAllocated_ += numPages;
      MOZ_ASSERT(pagesAllocated_ <= MaxCodePages);

      // If we allocated a small number of pages, move cursor_ to the
      // next page. We don't do this for larger allocations to avoid
      // skipping a large number of small holes.
      if (numPages <= 2) {
        cursor_ = page + numPages;
      }

      p = base_ + page * ExecutableCodePageSize;
      break;
    }
    if (!p) {
      return nullptr;
    }
  }

  // Commit the pages after releasing the lock.
  if (!CommitPages(p, bytes, protection)) {
    deallocate(p, bytes, /* decommit = */ false);
    return nullptr;
  }

  SetMemCheckKind(p, bytes, checkKind);

  return p;
}


void JitSandboxExecutableMemory::deallocate(void* addr, size_t bytes,
                                         bool decommit) {
  MOZ_ASSERT(initialized());
  MOZ_ASSERT(addr);
  MOZ_ASSERT((uintptr_t(addr) % gc::SystemPageSize()) == 0);
  MOZ_ASSERT(bytes > 0);
  MOZ_ASSERT((bytes % ExecutableCodePageSize) == 0);

  assertValidAddress(addr, bytes);

  size_t firstPage =
      (static_cast<uint8_t*>(addr) - base_) / ExecutableCodePageSize;
  size_t numPages = bytes / ExecutableCodePageSize;

  // Decommit before taking the lock.
  MOZ_MAKE_MEM_NOACCESS(addr, bytes);
  if (decommit) {
    DecommitPages(addr, bytes);
  }

  LockGuard<Mutex> guard(lock_);
  MOZ_ASSERT(numPages <= pagesAllocated_);
  pagesAllocated_ -= numPages;

  for (size_t i = 0; i < numPages; i++) {
    pages_.remove(firstPage + i);
  }

  // Move the cursor back so we can reuse pages instead of fragmenting the
  // whole region.
  if (firstPage < cursor_) {
    cursor_ = firstPage;
  }
}

static JitSandboxExecutableMemory execMemory;

bool js::jitsbx::ReprotectRegion(void* start, size_t size,
                              ProtectionSetting protection,
                              MustFlushICache flushICache) {

  // Flush ICache when making code executable, before we modify |size|.
  if (flushICache == MustFlushICache::Yes) {
    MOZ_ASSERT(protection == ProtectionSetting::Executable);
    jit::FlushICache(start, size);
  }

  // Calculate the start of the page containing this region,
  // and account for this extra memory within size.
  size_t pageSize = gc::SystemPageSize();
  intptr_t startPtr = reinterpret_cast<intptr_t>(start);
  intptr_t pageStartPtr = startPtr & ~(pageSize - 1);
  void* pageStart = reinterpret_cast<void*>(pageStartPtr);
  size += (startPtr - pageStartPtr);

  // Round size up
  size += (pageSize - 1);
  size &= ~(pageSize - 1);

  MOZ_ASSERT((uintptr_t(pageStart) % pageSize) == 0);

  execMemory.assertValidAddress(pageStart, size);

  // On weak memory systems, make sure new code is visible on all cores before
  // addresses of the code are made public.  Now is the latest moment in time
  // when we can do that, and we're assuming that every other thread that has
  // written into the memory that is being reprotected here has synchronized
  // with this thread in such a way that the memory writes have become visible
  // and we therefore only need to execute the fence once here.  See bug 1529933
  // for a longer discussion of why this is both necessary and sufficient.
  //
  // We use the C++ fence here -- and not AtomicOperations::fenceSeqCst() --
  // primarily because ReprotectRegion will be called while we construct our own
  // jitted atomics.  But the C++ fence is sufficient and correct, too.
  std::atomic_thread_fence(std::memory_order_seq_cst);

  unsigned flags = ProtectionSettingToFlags(protection);
  if (mprotect(pageStart, size, flags)) {
    return false;
  }

  execMemory.assertValidAddress(pageStart, size);
  return true;
}

void* js::jitsbx::AllocateExecutableMemory(size_t bytes,
                                        ProtectionSetting protection,
                                        MemCheckKind checkKind) {
  return execMemory.allocate(bytes, protection, checkKind);
}

void js::jitsbx::DeallocateExecutableMemory(void* addr, size_t bytes) {
  execMemory.deallocate(addr, bytes, /* decommit = */ true);
}

bool js::jitsbx::InitExecutableMemory() { return execMemory.init(); }

void js::jitsbx::ReleaseExecutableMemory() { execMemory.release(); }

size_t js::jitsbx::LikelyAvailableExecutableMemory() {
  // Round down available memory to the closest MB.
  return MaxCodeBytesPerProcess -
         AlignBytes(execMemory.bytesAllocated(), 0x100000U);
}

bool js::jitsbx::CanLikelyAllocateMoreExecutableMemory() {
  // Use a 8 MB buffer.
  static const size_t BufferSize = 8 * 1024 * 1024;

  MOZ_ASSERT(execMemory.bytesAllocated() <= MaxCodeBytesPerProcess);

  return execMemory.bytesAllocated() + BufferSize <= MaxCodeBytesPerProcess;
}

bool js::jitsbx::AddressIsInExecutableMemory(const void* p) {
  return execMemory.containsAddress(p);
}
