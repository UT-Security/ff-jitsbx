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
#include "monkeycage/RootingAPI.h"

struct MCRuntime {
  MCRuntime* parent_;
  JSRuntime* rt_;
};

struct MCContext : MC::RootingContext {
  JSContext* cx_;
  void* data_;

  MCRuntime* rt_;

  static inline thread_local MCContext* mcx_;
};

inline JSContext* MC_UNSAFE(MCContext* cx) {
  return cx->cx_;
}

inline JSRuntime* MC_UNSAFE(MCRuntime* rt) {
  return rt->rt_;
}

inline MCContext* MC_NewContext(uint32_t maxbytes, MCRuntime* parentRuntime = nullptr) { 
  MOZ_RELEASE_ASSERT(!MCContext::mcx_, "Attempt to create duplication MCContext in thread");

  JSContext* jscx = JS_NewContext(maxbytes, parentRuntime == nullptr ? nullptr : parentRuntime->rt_);
  if (!jscx) {
    return nullptr;
  }

  JSRuntime* jsrt = JS_GetRuntime(jscx);
  if (!jsrt) {
    return nullptr;
  }
  
  MCContext* cx = new MCContext();
  MOZ_RELEASE_ASSERT(cx, "MCContext allocation failed!");

  MCRuntime* rt = new MCRuntime();
  MOZ_RELEASE_ASSERT(rt, "MCRuntime allocation failed!");

  rt->parent_ = parentRuntime;
  rt->rt_ = jsrt;

  cx->cx_ = jscx;
  cx->data_ = nullptr;
  cx->rt_ = rt;

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
  delete cx->rt_;
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

inline MCRuntime* JS_GetParentRuntime(MCContext* cx) {
  return cx->rt_->parent_;
}

inline MCRuntime* JS_GetRuntime(MCContext* cx) {
  return cx->rt_;
}

inline void JS_SetFutexCanWait(MCContext* cx) {
  return JS_SetFutexCanWait(cx->cx_);
}
#else
using MCRuntime = JSRuntime;
using MCContext = JSContext;

inline JSContext* MC_UNSAFE(MCContext* cx) {
  return cx;
}

inline JSRuntime* MC_UNSAFE(MCRuntime* rt) {
  return rt;
}

inline MCContext* MC_NewContext(uint32_t maxbytes, JSRuntime* parentRuntime = nullptr) { 
  return JS_NewContext(maxbytes, parentRuntime);
}

inline MCContext* JS_SanitizeContext(JSContext* cx) {
  return cx;
}

#endif


#endif
