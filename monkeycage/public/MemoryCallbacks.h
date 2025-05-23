/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_MemoryCallbacks_h
#define mc_MemoryCallbacks_h

#include "js/MemoryCallbacks.h"

#ifdef JS_SANDBOX

#include "monkeycage/SandboxCallback.h"

struct MCContext;

namespace JS {

inline void SetOutOfMemoryCallback(MCContext* cx, MC::SandboxCallback<OutOfMemoryCallback> cb,
                                   void* data) {
  return SetOutOfMemoryCallback(cx->cx_, cb.UNSAFE_get(), data);  
}

}
#endif

#endif
