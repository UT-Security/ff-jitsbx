/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_WaitCallbacks_h
#define mc_WaitCallbacks_h

#include "js/WaitCallbacks.h"

#ifdef JS_SANDBOX

#include "monkeycage/SandboxCallback.h"

namespace JS {
inline void SetWaitCallback(JSRuntime* rt, MC::SandboxCallback<BeforeWaitCallback> beforeWait,
                            MC::SandboxCallback<AfterWaitCallback> afterWait, size_t requiredMemory) {
  return SetWaitCallback(rt, beforeWait.UNSAFE_get(), afterWait.UNSAFE_get(), requiredMemory);
}
}
#endif

#endif
