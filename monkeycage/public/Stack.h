/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Stack_h
#define mc_Stack_h

#include "js/Stack.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxStack.h"
#include "monkeycage/Tainted.h"

inline void JS_SetNativeStackQuota(
    MCContext* cx, JS::NativeStackSize systemCodeStackSize,
    JS::NativeStackSize trustedScriptStackSize = 0,
    JS::NativeStackSize untrustedScriptStackSize = 0) {
  JS_SetNativeStackQuota(cx->cx_, systemCodeStackSize, trustedScriptStackSize, untrustedScriptStackSize);
}

namespace JS {

inline bool CaptureCurrentStack(
    MCContext* cx, MutableHandleObject stackp,
    StackCapture&& capture = StackCapture(AllFrames())) {
    MC::SandboxStack<StackCapture> captureSbx(std::forward<StackCapture&&>(capture));
  return CaptureCurrentStack(cx->cx_, stackp, captureSbx.UNSAFE_unverified());
}

inline bool BuildStackString(
    MCContext* cx, JSPrincipals* principals, HandleObject stack,
    MutableHandleString stringp, size_t indent = 0,
    js::StackFormat stackFormat = js::StackFormat::Default) {
  return BuildStackString(cx->cx_, principals, stack, stringp, indent,
                          stackFormat);
}
}

#endif

#endif
