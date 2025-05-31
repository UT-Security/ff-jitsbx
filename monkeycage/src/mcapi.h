/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript API. */

#ifndef mcapi_h
#define mcapi_h

#include "jsapi.h"

#include "monkeycage/Context.h"
#include "monkeycage/ErrorReport.h"
#include "monkeycage/Exception.h"
#include "monkeycage/GCAPI.h"
#include "monkeycage/GCVector.h"
#include "monkeycage/GlobalObject.h"
#include "monkeycage/Interrupt.h"
#include "monkeycage/MemoryCallbacks.h"
#include "monkeycage/Realm.h"
#include "monkeycage/RealmIterators.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/ScriptPrivate.h"
#include "monkeycage/Stack.h"
#include "monkeycage/String.h"
#include "monkeycage/TypeDecls.h"
#include "monkeycage/UniquePtr.h"
#include "monkeycage/Utility.h"
#include "monkeycage/Value.h"
#include "monkeycage/ValueArray.h"
#include "monkeycage/WaitCallbacks.h"
#include "monkeycage/WrapperCallbacks.h"

#include "monkeycage/Tainted.h"

#ifdef JS_SANDBOX

inline void JS_SetWrapObjectCallbacks(
    MCContext* cx, const MCWrapObjectCallbacks* callbacks) {
  JS_SetWrapObjectCallbacks(cx->cx_, callbacks->UNSAFE_get());
}

inline bool JS_WrapObject(MCContext* cx, MC::MutableHandleObject objp) {
  return JS_WrapObject(cx->cx_, objp.MC_INTERNAL_SAFE_get());
}

inline bool JS_WrapValue(MCContext* cx, MC::MutableHandleValue vp) {
  return JS_WrapValue(cx->cx_, vp.MC_INTERNAL_SAFE_get());
}

inline void JS_SetParallelParsingEnabled(MCContext* cx, bool enabled) {
  return JS_SetParallelParsingEnabled(cx->cx_, enabled);
}

inline void JS_SetOffthreadIonCompilationEnabled(MCContext* cx,
                                                               bool enabled) {
  return JS_SetOffthreadIonCompilationEnabled(cx->cx_, enabled);
}

inline void JS_SetGlobalJitCompilerOption(MCContext* cx,
                                          JSJitCompilerOption opt,
                                          uint32_t value) {
  return JS_SetGlobalJitCompilerOption(cx->cx_, opt, value);  
}

inline bool JS_GetGlobalJitCompilerOption(MCContext* cx,
                                          JSJitCompilerOption opt,
                                          uint32_t* valueOut) {
  return JS_GetGlobalJitCompilerOption(cx->cx_, opt, valueOut);
}

namespace JS {
inline void HideScriptedCaller(MCContext* cx) {
  return HideScriptedCaller(cx->cx_);
}

inline void UnhideScriptedCaller(MCContext* cx) {
  return UnhideScriptedCaller(cx->cx_);
}
}
#endif

#endif
