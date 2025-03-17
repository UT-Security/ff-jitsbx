/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Initialization_h
#define monkeycage_Initialization_h

#include "js/Initialization.h"

#include "monkeycage/Sandbox.h"
#include "monkeycage/Tainted.h"

using namespace monkeycage;

typedef Tainted<void*> (*MCJS_ICUAllocFn)(TaintedUnchecked<const void*>, Tainted<size_t> size);
typedef Tainted<void*> (*MCJS_ICUReallocFn)(TaintedUnchecked<const void*>, TaintedUnchecked<void*> p, Tainted<size_t> size);
typedef void (*MCJS_ICUFreeFn)(TaintedUnchecked<const void*>, TaintedUnchecked<void*> p);

bool MCJS_SetICUMemoryFunctions(SandboxCallback<MCJS_ICUAllocFn> allocFn,
                                SandboxCallback<MCJS_ICUReallocFn> reallocFn,
                                SandboxCallback<MCJS_ICUFreeFn> freeFn) {
  return JS_SetICUMemoryFunctions(reinterpret_cast<JS_ICUAllocFn>(allocFn.get()),
                                  reinterpret_cast<JS_ICUReallocFn>(reallocFn.get()),
                                  reinterpret_cast<JS_ICUFreeFn>(freeFn.get()));
}

#endif
