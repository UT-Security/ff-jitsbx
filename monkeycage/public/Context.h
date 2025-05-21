/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Monkeycage API. */

#ifndef mc_Context_h
#define mc_Context_h

#include "js/Context.h"

#ifdef JS_SANDBOX

#include "mozilla/Assertions.h"

struct MCContext {
  JSContext* cx_;
  void* data_;

  static inline thread_local MCContext* mcx_;
};

inline JSContext* MC_UNSAFE(MCContext* cx) {
  return cx->cx_;
}

inline MCContext* MC_NewContext(uint32_t maxbytes, JSRuntime* parentRuntime = nullptr) { 
  MOZ_RELEASE_ASSERT(!MCContext::mcx_, "Attempt to create duplication MCContext in thread");

  JSContext* jscx = JS_NewContext(maxbytes, parentRuntime);
  if (!jscx) {
    return nullptr;
  }
  
  MCContext* cx = new MCContext();
  MOZ_RELEASE_ASSERT(cx, "MCContext allocation failed!");

  cx->cx_ = jscx;
  cx->data_ = nullptr;

  MCContext::mcx_ = cx;
  return cx;
}

inline MCContext* JS_SanitizeContext(JSContext* cx) {
  MOZ_RELEASE_ASSERT(MCContext::mcx_);
  MOZ_RELEASE_ASSERT(MCContext::mcx_->cx_ == cx);
  return MCContext::mcx_;
}

inline void JS_DestroyContext(MCContext* cx) {
  MOZ_RELEASE_ASSERT(MCContext::mcx_, "Attempt to delete MCContext in non-allocating thread");
  MOZ_RELEASE_ASSERT(MCContext::mcx_ == cx, "Attempt to delete MCConxtext from different thread");

  JS_DestroyContext(cx->cx_);
  delete cx;  
  MCContext::mcx_ = nullptr;
}

inline void* JS_GetContextPrivate(MCContext* cx) {
  return cx->data_;
}

inline void JS_SetContextPrivate(MCContext* cx, void* data) { 
  //TODO(abhishekcs): duplication exists for now until all of Gecko uses MCContext* and this function.
  cx->data_ = data;
  JS_SetContextPrivate(cx->cx_, data);
}

inline JSRuntime* JS_GetParentRuntime(MCContext* cx) {
  return JS_GetParentRuntime(cx->cx_);
}

inline JSRuntime* JS_GetRuntime(MCContext* cx) {
  return JS_GetRuntime(cx->cx_);
}

inline void JS_SetFutexCanWait(MCContext* cx) {
  return JS_SetFutexCanWait(cx->cx_);
}
#else
using MCContext = JSContext;

inline JSContext* MC_UNSAFE(MCContext* cx) {
  return cx;
}

inline MCContext* MC_NewContext(uint32_t maxbytes, JSRuntime* parentRuntime = nullptr) { 
  return JS_NewContext(maxbytes, parentRuntime);
}

inline MCContext* JS_SanitizeContext(JSContext* cx) {
  return cx;
}

#endif


#endif
