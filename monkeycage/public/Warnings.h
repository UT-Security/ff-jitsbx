/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_Warnings_h
#define mc_Warnings_h

#include "js/Warnings.h"

#ifdef JS_SANDBOX
#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"
#include "monkeycage/SandboxCallback.h"

namespace JS {

template <typename... Args>
inline bool WarnASCII(MCContext* cx, const char* format, Args... args) {
  return WarnASCII(cx->cx_, format, args...);
}

template <typename... Args>
inline bool WarnLatin1(MCContext* cx, const char* format, Args... args) {
  return WarnLatin1(cx->cx_, format, args...);
}

template <typename... Args>
inline bool WarnUTF8(MCContext* cx, const char* format, Args... args) {
  return WarnUTF8(cx->cx_, format, args...);
}

inline MC::SandboxCallback<WarningReporter> GetWarningReporter(MCContext* cx) {
  auto UNSAFE_callback = GetWarningReporter(cx->cx_);
  return MC::Sandbox::RetrieveCallback(UNSAFE_callback);
}

inline MC::SandboxCallback<WarningReporter> SetWarningReporter(
    MCContext* cx, MC::SandboxCallback<WarningReporter> reporter) {
  auto UNSAFE_callback =  SetWarningReporter(cx->cx_, reporter.UNSAFE_get());
  return MC::Sandbox::RetrieveCallback(UNSAFE_callback);
}
}  // namespace JS
#endif

#endif
