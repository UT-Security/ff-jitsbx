/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jitsbx_JitSandboxMemory_h
#define jitsbx_JitSandboxMemory_h

#include <atomic>
#include <stddef.h>
#include <stdint.h>


namespace js {

namespace jitsbx {

static constexpr uintptr_t JITSBX_HEAP_BASE = (uintptr_t)1 << 32; 
static constexpr size_t JITSBX_HEAP_SIZE = (size_t)1 << 32;
  
extern std::atomic<uint64_t> heapBumpPtr;

void InitHeapMemory();
void* MapAlignedPages(size_t length, size_t alignment);
void UnmapPages(void* region, size_t length);
  
};

};

#endif
