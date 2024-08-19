/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef sandbox_Memory_h
#define sandbox_Memory_h

#include <stddef.h>
#include <stdint.h>

#include "sandbox/DoublyLinkedList.h"
#include "sandbox/RBTree.h"

namespace js {
  
namespace sandbox {

static constexpr uint64_t MemoryMask = ((UINT64_C(1) << 40) - 1);
static constexpr uint32_t MemoryShift = (64 - 40);

struct MemoryRegion;

class Memory {
private:
  RBTree<MemoryRegion, MemoryRegion> region_tree;

  DoublyLinkedList<MemoryRegion> region_list;

  uintptr_t base_addr;

  size_t length;

  // Unit of Region allocation and alignment.
  size_t page_size;

  // Highest Memory Region end address.
  uintptr_t highest_region_end;


  uintptr_t findUnmappedRegion(size_t len, size_t align);
  uintptr_t findUnmappedEndRegion(size_t len);
  
  int findRegionLinks(uintptr_t start_addr, uintptr_t end_addr,
                      MemoryRegion** pprev, MemoryRegion*** link,
                      MemoryRegion** parent);

  MemoryRegion* findRegion(uintptr_t addr);
  MemoryRegion* findRegionAndPrev(uintptr_t addr, MemoryRegion** pprev);
  MemoryRegion* findIntersectingRegion(uintptr_t start_addr,
                                       uintptr_t end_addr);

  MemoryRegion* lookupRegion(uintptr_t addr);
  MemoryRegion* nextRegion(MemoryRegion* r);

  MemoryRegion* maybeMergeRegions(MemoryRegion* prev, uintptr_t start_addr,
                                 uintptr_t end_addr);
  void adjustRegion(MemoryRegion* r, uintptr_t start_addr, uintptr_t end_addr);

  void linkRegion(MemoryRegion *r, MemoryRegion *prev, MemoryRegion **link, MemoryRegion *parent);
  void linkRegionList(MemoryRegion *r, MemoryRegion *prev);
  void linkRegionTree(MemoryRegion *r, MemoryRegion **link, MemoryRegion *parent);
  void unlinkRegion(MemoryRegion *r, MemoryRegion *ignore);
  void unlinkRegionList(MemoryRegion *r);

 public:
  Memory() : base_addr((uintptr_t)nullptr), length(0), page_size(0) {}

  bool initialized() { return base_addr != (uintptr_t)nullptr; }

  bool init(void* addr, size_t length, size_t pageSize);

  uintptr_t base() { return (uintptr_t)base_addr; }
  void* allocateProtected(size_t length, size_t alignment);
  void* allocate(size_t length, size_t alignment);
  void deallocateProtected(void* addr, size_t length);
  void deallocate(void* addr, size_t length);
};

bool InitMemory();

uintptr_t MemoryBase();
void* AllocateProtectedMemory(size_t length, size_t alignment);
void* AllocateMemory(size_t length, size_t alignment);
void DeallocateProtectedMemory(void* addr, size_t length);
void  DeallocateMemory(void* addr, size_t length);

} /* namespace sandbox */
  
} /* namespace js */


#endif /* sandbox_MemoryRegion_h */
