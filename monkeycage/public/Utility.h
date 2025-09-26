/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Utility_h
#define mc_Utility_h

#include "js/Utility.h"

namespace mc {
extern arena_id_t GetMallocArena();
extern arena_id_t GetArrayBufferContentsArena();
extern arena_id_t GetStringBufferArena();
}

#ifdef JS_SANDBOX

#include "monkeycage/SandboxCallback.h"
#include "monkeycage/Tainted.h"

namespace mc {
inline void setAnnotateOOMAllocationSizeCallback(
    MC::SandboxCallback<
        js::AutoEnterOOMUnsafeRegion::AnnotateOOMAllocationSizeCallback>
        callback) {
  js::AutoEnterOOMUnsafeRegion::setAnnotateOOMAllocationSizeCallback(
      callback.UNSAFE_get());
}
}  // namespace mc


template <class T>
static inline void mc_free(MC::Tainted<T*> p) {
    if (p) {
        js_free(p.INTERNAL_unverified_safe());
    }
}

template <class T, typename... Args>
static inline MC::Tainted<T*> MOZ_HEAP_ALLOCATOR mc_new(Args&&... args) {
    MC::Tainted<T*> ret{nullptr};
    T* ptr = js_new<T>(std::forward<Args>(args)...);
    ret.assign_raw_pointer(ptr);
    return ret;
}

#else
namespace mc {
  
inline void setAnnotateOOMAllocationSizeCallback(js::AutoEnterOOMUnsafeRegion::AnnotateOOMAllocationSizeCallback
        callback) {
  js::AutoEnterOOMUnsafeRegion::setAnnotateOOMAllocationSizeCallback(
      callback);
}
}  // namespace mc

#endif

#endif
