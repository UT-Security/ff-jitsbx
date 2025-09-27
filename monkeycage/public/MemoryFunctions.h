/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Low-level memory-allocation functions. */

#ifndef mc_MemoryFunctions_h
#define mc_MemoryFunctions_h

#include "js/MemoryFunctions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

MC::Tainted<void*> JS_malloc(MCContext* cx, size_t nbytes) {
  MC::Tainted<void*> ret{nullptr};
  void* ptr = JS_malloc(cx->cx_, nbytes);
  ret.assign_raw_pointer(ptr);
  return ret;
}

#endif

#endif
