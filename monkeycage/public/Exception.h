/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Exception_h
#define mc_Exception_h

#include "js/Exception.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/Tainted.h"
#include "monkeycage/TypeDecls.h"

namespace JS {

inline bool GetPendingExceptionStack(MCContext* cx, MC::Tainted<ExceptionStack*> exceptionStack) {
  return GetPendingExceptionStack(cx->cx_, exceptionStack.UNSAFE_unverified());
}

inline bool StealPendingExceptionStack(MCContext* cx,
                                       MC::Tainted<ExceptionStack*> exceptionStack) {
  return StealPendingExceptionStack(cx->cx_, exceptionStack.UNSAFE_unverified());  
}

inline void SetPendingExceptionStack(
    MCContext* cx, MC::Tainted<const ExceptionStack*> exceptionStack) {
  return SetPendingExceptionStack(cx->cx_, *exceptionStack.UNSAFE_unverified());
}

}

inline bool JS_IsExceptionPending(MCContext* cx) {
  return JS_IsExceptionPending(cx->cx_);
}

inline bool JS_IsThrowingOutOfMemory(MCContext* cx) {
  return JS_IsThrowingOutOfMemory(cx->cx_);
}

inline bool JS_GetPendingException(MCContext* cx, MC::MutableHandleValue vp) {
  return JS_GetPendingException(cx->cx_, vp.MC_INTERNAL_SAFE_get());
}

inline void JS_SetPendingException(
    MCContext* cx, JS::HandleValue v,
    JS::ExceptionStackBehavior behavior = JS::ExceptionStackBehavior::Capture) {
  return JS_SetPendingException(cx->cx_, v, behavior);
}

inline void JS_ClearPendingException(MCContext* cx) {
  JS_ClearPendingException(cx->cx_);
}

inline JSErrorReport* JS_ErrorFromException(MCContext* cx,
                                            JS::HandleObject obj) {
  return JS_ErrorFromException(cx->cx_, obj);
}
#endif
#endif
