/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Low-level memory-allocation functions. */

#ifndef mc_MemoryFunctions_h
#define mc_MemoryFunctions_h

#include "js/MemoryFunctions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

static inline MC::Tainted<void*> JS_malloc(MCContext* cx, size_t nbytes) {
  MC::Tainted<void*> ret{nullptr};
  void* ptr = JS_malloc(cx->cx_, nbytes);
  ret.assign_raw_pointer(ptr);
  return ret;
}

static inline MC::Tainted<void*> JS_realloc(MCContext* cx, MC::Tainted<void*> p, size_t oldBytes,
                                      size_t newBytes) {
  MC::Tainted<void*> ret{nullptr};
  void* ptr = JS_realloc(cx->cx_, p.INTERNAL_unverified_safe(), oldBytes, newBytes);
  ret.assign_raw_pointer(ptr);
  return ret;
}

static inline MC::Tainted<void*> JS_string_malloc(MCContext* cx, size_t nbytes) {
  MC::Tainted<void*> ret{nullptr};
  void* ptr = JS_string_malloc(cx->cx_, nbytes);
  ret.assign_raw_pointer(ptr);
  return ret;
}

static inline MC::Tainted<void*> JS_string_realloc(MCContext* cx, MC::Tainted<void*> p,
                                             size_t oldBytes, size_t newBytes) {
  MC::Tainted<void*> ret{nullptr};
  void* ptr = JS_string_realloc(cx->cx_, p.INTERNAL_unverified_safe(), oldBytes, newBytes);
  ret.assign_raw_pointer(ptr);
  return ret;
}

static inline void JS_string_free(MCContext* cx, MC::Tainted<void*> p) {
  JS_string_free(cx->cx_, p.INTERNAL_unverified_safe());  
}

#endif

#endif
