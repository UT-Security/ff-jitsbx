/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* SpiderMonkey initialization and shutdown APIs. */

#ifndef mc_Initialization_h
#define mc_Initialization_h

#include "SandboxCallback.h"
#include "js/Initialization.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {
  
inline bool InitSelfHostedCode(MCContext* cx,
                                      SelfHostedCache cache = nullptr,
                                      SelfHostedWriter writer = nullptr) {
  return InitSelfHostedCode(cx->cx_, cache, writer);
}
}

inline bool JS_SetICUMemoryFunctions(
    MC::SandboxCallback<JS_ICUAllocFn> allocFn,
    MC::SandboxCallback<JS_ICUReallocFn> reallocFn,
    MC::SandboxCallback<JS_ICUFreeFn> freeFn) {
  return JS_SetICUMemoryFunctions(allocFn.UNSAFE_get(), reallocFn.UNSAFE_get(),
                                  freeFn.UNSAFE_get());
}

#endif

#endif
