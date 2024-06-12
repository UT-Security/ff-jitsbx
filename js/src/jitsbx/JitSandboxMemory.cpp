/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jitsbx/JitSandboxMemory.h"
#include "mozilla/Assertions.h"
#include "mozilla/TaggedAnonymousMemory.h"

#include <sys/mman.h>
#include <unistd.h>

#include "js/HeapAPI.h"

namespace js {

namespace jitsbx {
  
static size_t pageSize = 0;
std::atomic<uint64_t> heapBumpPtr;

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

void* MapAlignedPages(size_t length, size_t alignment) {
  MOZ_ASSERT(length % alignment == 0);
  MOZ_ASSERT(alignment % js::gc::ChunkSize == 0);

  void* currentPtr = (void*)heapBumpPtr.fetch_add(length);
  MOZ_ASSERT((uint64_t)currentPtr >> 32 == heapBumpPtr >> 32);
  MOZ_ASSERT(currentPtr != 0);

  UnprotectPages(currentPtr, length);
  return currentPtr;
}

void InitHeapMemory() {
  pageSize = size_t(sysconf(_SC_PAGESIZE));
  heapBumpPtr = (uint64_t)MapInternal<Commit::No, PageAccess::None>((void*)JITSBX_HEAP_BASE, JITSBX_HEAP_SIZE);
  // Initialize gsbase
  __asm__ __volatile__("wrgsbase %0": : "r" (JITSBX_HEAP_BASE));
}

};
  
};
