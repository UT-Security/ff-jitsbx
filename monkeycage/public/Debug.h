/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Interfaces by which the embedding can interact with the Debugger API.

#ifndef mc_Debug_h
#define mc_Debug_h

#include "js/Debug.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

inline bool JS_DefineDebuggerObject(MCContext* cx, JS::HandleObject obj) {
  return JS_DefineDebuggerObject(cx->cx_, obj);  
}

namespace JS {
namespace dbg {
  
inline void SetDebuggerMallocSizeOf(MCContext* cx,
                                           mozilla::MallocSizeOf mallocSizeOf) {
  SetDebuggerMallocSizeOf(cx->cx_, mallocSizeOf);
}

inline mozilla::MallocSizeOf GetDebuggerMallocSizeOf(MCContext* cx) {
  return GetDebuggerMallocSizeOf(cx->cx_);
}

}
}  // namespace JS

#endif

#endif
