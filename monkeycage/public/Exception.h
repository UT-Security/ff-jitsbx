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

namespace MC {
class MOZ_STACK_CLASS ExceptionStack {
 public:
  //TODO(abhishek): this should be a private member
  JS::ExceptionStack inner_;
  
  explicit ExceptionStack(MCContext* cx) : inner_(cx->cx_) {}

  ExceptionStack(MCContext* cx, JS::HandleValue exception, JS::HandleObject stack)
      : inner_(cx->cx_, exception, stack) {}

  JS::HandleValue exception() const { return inner_.exception(); }

  // |stack| can be null.
  JS::HandleObject stack() const { return inner_.stack(); }
};
}

namespace JS {

inline bool GetPendingExceptionStack(MCContext* cx, MC::ExceptionStack* exceptionStack) {
  return GetPendingExceptionStack(cx->cx_, &exceptionStack->inner_);
}

inline bool StealPendingExceptionStack(MCContext* cx,
                                       MC::ExceptionStack* exceptionStack) {
  return StealPendingExceptionStack(cx->cx_, &exceptionStack->inner_);  
}

inline void SetPendingExceptionStack(
    MCContext* cx, const MC::ExceptionStack& exceptionStack) {
  return SetPendingExceptionStack(cx->cx_, exceptionStack.inner_);
}

}

inline bool JS_IsExceptionPending(MCContext* cx) {
  return JS_IsExceptionPending(cx->cx_);
}

inline void JS_ClearPendingException(MCContext* cx) {
  JS_ClearPendingException(cx->cx_);
}

#else
namespace MC {

using ExceptionStack = JS::ExceptionStack;

}
#endif

#endif
