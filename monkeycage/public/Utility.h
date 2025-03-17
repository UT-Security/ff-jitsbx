/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Utility_h
#define monkeycage_Utility_h

#include "js/Utility.h"
#include "jstypes.h"

namespace monkeycage {}

namespace monkeycage {

extern "C" void* sbx_moz_arena_malloc(arena_id_t, size_t);
extern "C" void* sbx_moz_arena_calloc(arena_id_t, size_t, size_t);
extern "C" void* sbx_moz_arena_realloc(arena_id_t, void*, size_t);
extern "C" void sbx_free(void*);

static inline void* js_arena_malloc(arena_id_t arena, size_t bytes) {
  JS_OOM_POSSIBLY_FAIL();
  JS_CHECK_LARGE_ALLOC(bytes);
  return sbx_moz_arena_malloc(arena, bytes);
}

static inline void* js_malloc(size_t bytes) {
  return js_arena_malloc(js::GetMallocArena(), bytes);
}

static inline void* js_arena_calloc(arena_id_t arena, size_t bytes) {
  JS_OOM_POSSIBLY_FAIL();
  JS_CHECK_LARGE_ALLOC(bytes);
  return sbx_moz_arena_calloc(arena, bytes, 1);
}

static inline void* js_arena_calloc(arena_id_t arena, size_t nmemb,
                                    size_t size) {
  JS_OOM_POSSIBLY_FAIL();
  JS_CHECK_LARGE_ALLOC(nmemb * size);
  return sbx_moz_arena_calloc(arena, nmemb, size);
}

static inline void* js_calloc(size_t bytes) {
  return js_arena_calloc(js::GetMallocArena(), bytes);
}

static inline void* js_calloc(size_t nmemb, size_t size) {
  return js_arena_calloc(js::GetMallocArena(), nmemb, size);
}

static inline void* js_arena_realloc(arena_id_t arena, void* p, size_t bytes) {
  // realloc() with zero size is not portable, as some implementations may
  // return nullptr on success and free |p| for this.  We assume nullptr
  // indicates failure and that |p| is still valid.
  MOZ_ASSERT(bytes != 0);

  JS_OOM_POSSIBLY_FAIL();
  JS_CHECK_LARGE_ALLOC(bytes);
  return sbx_moz_arena_realloc(arena, p, bytes);
}

static inline void* js_realloc(void* p, size_t bytes) {
  return js_arena_realloc(js::GetMallocArena(), p, bytes);
}

static inline void js_free(void* p) {
  sbx_free(p);
}

}  // namespace monkeycage

#endif
