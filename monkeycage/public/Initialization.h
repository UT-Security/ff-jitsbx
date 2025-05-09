/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* SpiderMonkey initialization and shutdown APIs. */

#ifndef mc_Initialization_h
#define mc_Initialization_h

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
#endif

#endif
