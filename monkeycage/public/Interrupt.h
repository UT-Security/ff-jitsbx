/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Interrupt_h
#define mc_Interrupt_h

#include "js/Interrupt.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

inline bool JS_CheckForInterrupt(MCContext* cx) {
  return JS_CheckForInterrupt(cx->cx_);
}

inline bool JS_AddInterruptCallback(
    MCContext* cx, MC::SandboxCallback<JSInterruptCallback> callback) {
  return JS_AddInterruptCallback(cx->cx_, callback.UNSAFE_get());
}

inline bool JS_DisableInterruptCallback(MCContext* cx) {
  return JS_DisableInterruptCallback(cx->cx_);
}

inline void JS_ResetInterruptCallback(MCContext* cx, bool enable) {
  return JS_ResetInterruptCallback(cx->cx_, enable);
}

inline void JS_RequestInterruptCallback(MCContext* cx) {
  return JS_RequestInterruptCallback(cx->cx_);
}

inline void JS_RequestInterruptCallbackCanWait(MCContext* cx) {
  return JS_RequestInterruptCallbackCanWait(cx->cx_);
}

#endif

#endif
