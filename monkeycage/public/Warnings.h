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
#include "monkeycage/SandboxCallback.h"

namespace JS {

inline WarningReporter GetWarningReporter(MCContext* cx) {
  return GetWarningReporter(cx->cx_);
}

inline WarningReporter SetWarningReporter(
    MCContext* cx, MC::SandboxCallback<WarningReporter> reporter) {
  return SetWarningReporter(cx->cx_, reporter.UNSAFE_get());
}
}  // namespace JS
#endif

#endif
