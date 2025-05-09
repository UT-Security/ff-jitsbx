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

struct MCContext {
  JSContext* cx_;
};

inline JSContext* MC_UNSAFE(MCContext* cx) {
  return cx->cx_;
}

inline MCContext* MC_NewContext(uint32_t maxbytes, JSRuntime* parentRuntime = nullptr) { 
  JSContext* jscx = JS_NewContext(maxbytes, parentRuntime);
  if (!jscx) {
    return nullptr;
  }
  
  MCContext* cx = new MCContext();
  cx->cx_ = jscx;
  return cx;
}

inline void JS_DestroyContext(MCContext* cx) {
  JS_DestroyContext(cx->cx_);
  delete cx;  
}
#else
using MCContext = JSContext;

inline JSContext* MC_UNSAFE(MCContext* cx) {
  return cx;
}

inline MCContext* MC_NewContext(uint32_t maxbytes, JSRuntime* parentRuntime = nullptr) { 
  return JS_NewContext(maxbytes, parentRuntime);
}

#endif


#endif
