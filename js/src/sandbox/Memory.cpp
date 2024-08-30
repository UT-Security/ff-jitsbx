/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sandbox/Memory.h"

#include "mozilla/Assertions.h"
#include "mozilla/MathAlgorithms.h"
#include "mozilla/RandomNum.h"
#include "mozilla/TaggedAnonymousMemory.h"

#include <algorithm>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "sandbox/DoublyLinkedList.h"
#include "sandbox/RBTree.h"

#define __ALIGN(x, a)         __ALIGN_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a)           __ALIGN((x), (a))
#define ALIGN_DOWN(x, a)      __ALIGN((x) - ((a) - 1), (a))

namespace js {
namespace sandbox {

/*
 * System allocation functions generally require the allocation size
 * to be an integer multiple of the page size of the running process.
 */
static size_t systemPageSize = 0;

/* The number of bits used by addresses on this platform. */
static size_t numAddressBits = 0;

/* The minimum and maximum valid addresses that can be used for memory reservations. */
static size_t minValidAddress = 0;
static size_t maxValidAddress = 0;

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

static inline size_t OffsetFromAligned(void* region, size_t alignment) {
  return uintptr_t(region) % alignment;
}

template <Commit commit, PageAccess prot>
static inline void* MapInternal(void* desired, size_t length) {
  void* region = nullptr;
  int flags = MAP_PRIVATE | MAP_ANON;
  
  region = mmap(desired, length, int(prot), flags, -1, 0);
  if (region == MAP_FAILED) {
    return nullptr;
  }
  
  return region;
}

static inline void UnmapInternal(void* region, size_t length) {
  //MOZ_ASSERT(region && OffsetFromAligned(region, systemPageSize) == 0);
  //MOZ_ASSERT(length > 0 && length % systemPageSize == 0);
  munmap(region, length);
}

static inline void ProtectMemory(void* region, size_t length, PageAccess prot) {
  //MOZ_RELEASE_ASSERT(region && OffsetFromAligned(region, systemPageSize) == 0);
  //MOZ_RELEASE_ASSERT(length > 0 && length % systemPageSize == 0);
  MOZ_RELEASE_ASSERT(mprotect(region, length, int(prot)) == 0); 
}

void ProtectPages(void* region, size_t length) {
  ProtectMemory(region, length, PageAccess::None);
}

void UnprotectPages(void* region, size_t length) {
  ProtectMemory(region, length, PageAccess::ReadWrite);
}

static Memory sandboxMemory;

struct MemoryRegion : RBTreeNode<MemoryRegion>, DoublyLinkedListElement<MemoryRegion> {
  friend class Memory;
  
  // First address within the region.
  uintptr_t start_addr;
  // First address after the region.
  uintptr_t end_addr;

  // Largest gap of free memory to the
  // left of this region.
  size_t subtree_gap;

  MemoryRegion(uintptr_t start_addr, uintptr_t end_addr) 
    : start_addr(start_addr),
      end_addr(end_addr) {}

  static inline void propagate(MemoryRegion* r, MemoryRegion* stop) {
    while (r != stop) {
      if (computeSubtreeMaxGap(r, true)) {
        break;
      }

      r = r->parent();
    }
  }

  static inline void copy(MemoryRegion* oldR, MemoryRegion* newR) {
    newR->subtree_gap = oldR->subtree_gap;
  }

  static inline void rotate(MemoryRegion* oldR, MemoryRegion* newR) {
    newR->subtree_gap = oldR->subtree_gap;
    computeSubtreeMaxGap(oldR, false);
  }

  static size_t computeGap(MemoryRegion* r) {
    size_t gap = r->start_addr;
    if (r->prev()) {
      size_t prevEnd = r->prev()->start_addr;
      if (gap > prevEnd) {
        gap -= prevEnd;
      } else {
        // WHY ?
        gap = 0;
      }
    }

    return gap;
  }

  static inline bool computeSubtreeMaxGap(MemoryRegion* r, bool exit) {
    size_t max = computeGap(r);

    if (r->left() && r->left()->subtree_gap > max) {
      max = r->left()->subtree_gap;
    }

    if (r->right() && r->right()->subtree_gap > max) {
      max = r->right()->subtree_gap;
    }

    if (exit && r->subtree_gap == max) {
      return true;
    }

    r->subtree_gap = max;
    return false;
  }
}; 

uintptr_t Memory::findUnmappedRegion(size_t len, size_t align) {
  

  // adjust length for worstcase alignment overhead.
  size_t aLen = len + (align - 1);
  //uintptr_t addr;

  if (region_tree.isEmpty()) {
    return ALIGN(findUnmappedEndRegion(aLen), align);    
  }
  
  MemoryRegion* r = region_tree.root();

  if (r->subtree_gap < aLen) {
    return ALIGN(findUnmappedEndRegion(aLen), align);
  }

  uintptr_t region_start, region_end;
  while (true) {
    region_end = r->start_addr;
    if (r->left() && r->left()->subtree_gap >= aLen) {
      r = r->left();
      continue;
    }

    region_start = r->prev() ? r->prev()->end_addr : base_addr;
check_current:
    if (region_end > region_start && (region_end - region_start) >= aLen) {
      return ALIGN(region_start, align);
    }

    if (r->right() && r->right()->subtree_gap >= aLen) {
      r = r->right();
      continue;
    }

    while (true) {
      MemoryRegion* prev = r;
      if (!prev->parent()) {
        return ALIGN(findUnmappedEndRegion(aLen), align);
      }

      r = prev->parent();
      if (prev == r->left()) {
        region_start = r->prev()->end_addr;
        region_end = r->start_addr;
        goto check_current;
      }
    }
  }

  //return ALIGN(addr, align);
}

uintptr_t Memory::findUnmappedEndRegion(size_t len) {
  if (highest_region_end + len < base_addr + length) {
    return highest_region_end;
  } else {
    return 0;
  }
}

// find the first MemoryRegion with addr < region->end_addr.
MemoryRegion* Memory::findRegion(uintptr_t addr) {
  MemoryRegion* r = region_tree.root();
  while (r) {
    if (r->end_addr > addr) {
      if (r->start_addr <= addr) {
        break;
      }
      r = r->left();
    } else {
      r = r->right();
    }
  }

  return r;
}

MemoryRegion* Memory::findRegionAndPrev(uintptr_t addr, MemoryRegion** pprev) {
  MemoryRegion* r = findRegion(addr);

  if (r) {
    *pprev = r->prev();
  } else {
    *pprev = region_tree.last();
  }

  return r;
}

inline MemoryRegion* Memory::findIntersectingRegion(uintptr_t start_addr,
                                                    uintptr_t end_addr) {
  MemoryRegion* r = findRegion(start_addr);

  if (r && end_addr < r->start_addr) {
    r = nullptr;
  }

  return r;
}

inline MemoryRegion* Memory::lookupRegion(uintptr_t addr) {
  MemoryRegion* r = findRegion(addr);

  if (r && addr < r->start_addr) {
    r = nullptr;
  }

  return r;
}


inline MemoryRegion* Memory::nextRegion(MemoryRegion* r) {
  if (!r) {
    return region_list.head();
  }

  return r->next();
}

int Memory::findRegionLinks(uintptr_t start_addr, uintptr_t end_addr,
                      MemoryRegion** pprev, MemoryRegion*** link,
                      MemoryRegion** parent) {
  MemoryRegion **link_ = region_tree.rootLink(), *parent_ = nullptr, *prev_ = nullptr;

  while (*link_) {
    parent_ = *link_;

    if (parent_->end_addr > start_addr) {
      if (parent_->start_addr < end_addr) {
        return -1;
      }
      link_ = parent_->leftLink();
    } else {
      prev_ = parent_;
      link_ = parent_->rightLink();
    }
  }

  *pprev = nullptr;
  if (prev_) {
    *pprev = prev_;
  }
  *link = link_;
  *parent = parent_;
  return 0;
}

MemoryRegion* Memory::maybeMergeRegions(MemoryRegion* prev, uintptr_t start_addr,
                                uintptr_t end_addr) {
  MemoryRegion* next = nextRegion(prev);

  // can we merge with predecessor?
  if (prev && prev->end_addr == start_addr) {
    // can we merge with successor too?
    if (next && end_addr == next->start_addr) {
      adjustRegion(prev, prev->start_addr, next->end_addr);
    } else {
      adjustRegion(prev, prev->start_addr, end_addr);
    }

    return prev;
  }

  // can we merge with successor?
  if (next && end_addr == next->start_addr) {
    adjustRegion(next, start_addr, next->end_addr);
    return next;
  }

  return nullptr;
}

void Memory::adjustRegion(MemoryRegion* r, uintptr_t start_addr, uintptr_t end_addr) {
  MemoryRegion* next = r->next();

  bool start_changed = false, end_changed = false, remove_next = false; 

  if (next && end_addr == next->end_addr) {
    remove_next = true;
  }

  if (start_addr != r->start_addr) {
    start_changed = true;
    r->start_addr = start_addr;
  }

  if (end_addr != r->end_addr) {
    end_changed = true;
    r->end_addr = end_addr; 
  }

  if (remove_next) {
    unlinkRegion(next, r);
    next = r->next();
    if (next) {
      MemoryRegion::propagate(next, nullptr);
    }
  } else if (start_changed) {
    MemoryRegion::propagate(r, nullptr);
  } else if (end_changed) {
    if (!next) {
      highest_region_end = end_addr;
    } else {
      MemoryRegion::propagate(next, nullptr);
    }
  }
}

void Memory::linkRegionList(MemoryRegion* r, MemoryRegion* prev) {
  MemoryRegion* next;

  r->setPrev(prev);
  if (prev) {
    next = prev->next();
    prev->setNext(r);
  } else {
    next = region_list.head();
    region_list.setHead(r);
  }

  r->setNext(next);
  if (next) {
    next->setPrev(r);
  }
}

void Memory::linkRegionTree(MemoryRegion *r, MemoryRegion **link, MemoryRegion *parent) {
  if (r->next()) {
    MemoryRegion::propagate(r->next(), nullptr);
  } else {
    highest_region_end = r->end_addr;
  }

  region_tree.link(r, parent, link);
  r->subtree_gap = 0;
  MemoryRegion::propagate(r, nullptr);
  region_tree.insert(r);
}

void Memory::linkRegion(MemoryRegion* r, MemoryRegion* prev,
                        MemoryRegion** link, MemoryRegion* parent) {
  linkRegionList(r, prev);
  linkRegionTree(r, link, parent);
}

inline void Memory::unlinkRegion(MemoryRegion* r, MemoryRegion* ignore) {
  region_tree.erase(r);

  MemoryRegion *next, *prev;

  next = r->next();
  prev = r->prev();

  if (prev) {
    prev->setNext(next);
  } else {
    region_list.setHead(next);
  }

  if (next) {
    next->setPrev(prev);
  }
}

static inline bool IsInvalidRegion(void* region, size_t length) {
  const uint64_t invalidPointerMask = UINT64_C(0xffff800000000000);
  return (uintptr_t(region) + length - 1) & invalidPointerMask;
}

bool Memory::init(void* addr, size_t length, size_t pageSize) {
  if (pageSize % systemPageSize != 0) {
    return false;
  }

  if (length % pageSize != 0) {
    return false;
  }

  if (IsInvalidRegion(addr, length)) {
    return false;
  }

  this->base_addr = (uintptr_t)addr;
  this->length = length; 
  this->page_size = pageSize;
  this->highest_region_end = (uintptr_t)addr;

  return true;
}

void* Memory::allocateProtected(size_t length, size_t alignment) {
  if (!initialized()) {
    return nullptr;
  }

  // check memory upper limit.
  if (length > this->length) {
    return nullptr;
  }

  // ensure requested alignment is aligned to page_size.
  if (alignment % page_size != 0) {
    return nullptr;
  }

  // ensure requested length is properly aligned.
  if (length % alignment != 0) {
    return nullptr;
  }

  // find an unallocated region large enough to accomodate the allocation.
  uintptr_t addr = findUnmappedRegion(length, alignment);
  if (!addr) {
    return nullptr;
  }

  MOZ_ASSERT(addr % systemPageSize == 0);
  MOZ_ASSERT(addr % alignment == 0);

  MemoryRegion *region = nullptr, *prev = nullptr, **link = nullptr, *parent = nullptr;

  // find the related/adjascent regions that need to be manipulated as part of
  // the allocation.
  findRegionLinks(addr, addr + length, &prev, &link, &parent);

  region = maybeMergeRegions(prev, addr, addr + length);
  if (region) {
    return (void*)addr;
  }

  void* memory = malloc(sizeof(MemoryRegion));
  if (!memory) {
    return nullptr;
  }
  region = new (memory) MemoryRegion(addr, addr + length);

  linkRegion(region, prev, link, parent);
  return (void*)addr;
}

void* Memory::allocate(size_t length, size_t alignment) {
  void* addr = allocateProtected(length, alignment);
  if (addr == nullptr) {
    return nullptr;
  }
  
  UnprotectPages((void*)addr, length);
  return (void*)addr;
}

void Memory::deallocateProtected(void* addr, size_t length) {
  MOZ_ASSERT(base_addr + ((uintptr_t)addr & MemoryMask) == (uintptr_t)addr);
  MOZ_ASSERT((uintptr_t)addr % page_size == 0);
  MOZ_ASSERT(length % page_size == 0);
}

void Memory::deallocate(void* addr, size_t length) {
  MOZ_ASSERT(base_addr + ((uintptr_t)addr & MemoryMask) == (uintptr_t)addr); 
  MOZ_ASSERT((uintptr_t)addr % page_size == 0);
  MOZ_ASSERT(length % page_size == 0);
  ProtectPages(addr, length);
  deallocateProtected(addr, length);
}

static inline uint64_t FindAddressLimitInner(size_t highBit, size_t tries);

/*
 * The address range available to applications depends on both hardware and
 * kernel configuration. For example, AArch64 on Linux uses addresses with
 * 39 significant bits by default, but can be configured to use addresses with
 * 48 significant bits by enabling a 4th translation table. Unfortunately,
 * there appears to be no standard way to query the limit at runtime
 * (Windows exposes this via GetSystemInfo()).
 *
 * This function tries to find the address limit by performing a binary search
 * on the index of the most significant set bit in the addresses it attempts to
 * allocate. As the requested address is often treated as a hint by the
 * operating system, we use the actual returned addresses to narrow the range.
 * We return the number of bits of an address that may be set.
 */
static size_t FindAddressLimit() {
  // Use 32 bits as a lower bound in case we keep getting nullptr.
  uint64_t low = 31;
  uint64_t highestSeen = (UINT64_C(1) << 32) - systemPageSize - 1;

  // Exclude 48-bit and 47-bit addresses first.
  uint64_t high = 47;
  for (; high >= std::max(low, UINT64_C(46)); --high) {
    highestSeen = std::max(FindAddressLimitInner(high, 4), highestSeen);
    low = mozilla::FloorLog2(highestSeen);
  }
  // If those didn't work, perform a modified binary search.
  while (high - 1 > low) {
    uint64_t middle = low + (high - low) / 2;
    highestSeen = std::max(FindAddressLimitInner(middle, 4), highestSeen);
    low = mozilla::FloorLog2(highestSeen);
    if (highestSeen < (UINT64_C(1) << middle)) {
      high = middle;
    }
  }
  // We can be sure of the lower bound, but check the upper bound again.
  do {
    high = low + 1;
    highestSeen = std::max(FindAddressLimitInner(high, 8), highestSeen);
    low = mozilla::FloorLog2(highestSeen);
  } while (low >= high);

  // `low` is the highest set bit, so `low + 1` is the number of bits.
  return low + 1;
}

/* Returns a random number in the given range. */
static inline uint64_t GetNumberInRange(uint64_t minNum, uint64_t maxNum) {
  const uint64_t MaxRand = UINT64_C(0xffffffffffffffff);
  maxNum -= minNum;
  uint64_t binSize = 1 + (MaxRand - maxNum) / (maxNum + 1);

  uint64_t rndNum;
  do {
    mozilla::Maybe<uint64_t> result;
    do {
      result = mozilla::RandomUint64();
    } while (!result);
    rndNum = result.value() / binSize;
  } while (rndNum > maxNum);

  return minNum + rndNum;
}

static void* MapAlignedPagesRandom(size_t length, size_t alignment) {
  uint64_t minNum = (minValidAddress + alignment - 1) / alignment;
  uint64_t maxNum = (maxValidAddress - (length - 1)) / alignment;

  // Try to allocate in random aligned locations.
  void* region = nullptr;
  for (size_t i = 1; i <= 1024; ++i) {
    uint64_t desired = alignment * GetNumberInRange(minNum, maxNum);
    region = MapInternal<Commit::No, PageAccess::None>(reinterpret_cast<void*>(desired), length);
    if (!region) {
      continue;
    }
    if (IsInvalidRegion(region, length)) {
      UnmapInternal(region, length);
      continue;
    }
    if (OffsetFromAligned(region, alignment) == 0) {
      return region;
    }
  }

  return region;
}

static inline uint64_t FindAddressLimitInner(size_t highBit, size_t tries) {
  const size_t length = systemPageSize;  // Used as both length and alignment.

  uint64_t highestSeen = 0;
  uint64_t startRaw = UINT64_C(1) << highBit;
  uint64_t endRaw = 2 * startRaw - length - 1;
  uint64_t start = (startRaw + length - 1) / length;
  uint64_t end = (endRaw - (length - 1)) / length;
  for (size_t i = 0; i < tries; ++i) {
    uint64_t desired = length * GetNumberInRange(start, end);
    void* address = MapInternal<Commit::Yes, PageAccess::ReadWrite>(reinterpret_cast<void*>(desired), length);
    uint64_t actual = uint64_t(address);
    if (address) {
      UnmapInternal(address, length);
    }
    if (actual > highestSeen) {
      highestSeen = actual;
      if (actual >= startRaw) {
        break;
      }
    }
  }
  return highestSeen;
}

bool InitMemory() {
  systemPageSize = size_t(sysconf(_SC_PAGESIZE));

  numAddressBits = FindAddressLimit();
  minValidAddress = UINT64_C(0x1) << 32;
  maxValidAddress = (UINT64_C(1) << numAddressBits) - 1 - systemPageSize;

  void* sandboxRegion = MapAlignedPagesRandom(MemorySize, MemorySize);
  if (sandboxRegion == nullptr) {
    return false;
  }

  MozTagAnonymousMemory(sandboxRegion, MemorySize, "js-sandbox-memory");


  if (!sandboxMemory.init(sandboxRegion, MemorySize, systemPageSize)) {
    return false;
  }

  return true;
}

uintptr_t MemoryBase() {
  return sandboxMemory.base();
}

bool IsValidAddress(size_t address) {
  return address >= sandboxMemory.base() && address < (sandboxMemory.base() + MemorySize);
}

void* AllocateProtectedMemory(size_t length, size_t alignment) {
  return sandboxMemory.allocateProtected(length, alignment);
}

void* AllocateMemory(size_t length, size_t alignment) {
  return sandboxMemory.allocate(length, alignment);
}

void DeallocateProtectedMemory(void* addr, size_t length) {
  sandboxMemory.deallocateProtected(addr, length);
}

void DeallocateMemory(void* addr, size_t length) {
  sandboxMemory.deallocate(addr, length);
}

}  // namespace sandbox
}  // namespace js
