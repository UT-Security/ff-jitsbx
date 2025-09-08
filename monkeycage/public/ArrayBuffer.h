/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* ArrayBuffer functionality. */

#ifndef mc_ArrayBuffer_h
#define mc_ArrayBuffer_h

#include "js/ArrayBuffer.h"

#ifdef JS_SANDBOX

#include "monkeycage/GCAPI.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline JSObject* CopyArrayBuffer(
    MCContext* cx, JS::Handle<JSObject*> maybeArrayBuffer) {
 return CopyArrayBuffer(cx->cx_, maybeArrayBuffer);
}

inline uint8_t* GetArrayBufferData(JSObject* obj, bool* isSharedMemory,
                                   const MC::Tainted<AutoCheckCannotGC*> nogc) {
  return GetArrayBufferData(obj, isSharedMemory, *nogc.UNSAFE_unverified());
}

}
#endif

#endif
