/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* ArrayBuffer functionality. */

#ifndef mc_ArrayBuffer_h
#define mc_ArrayBuffer_h

#include "js/ArrayBuffer.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline JSObject* GetObjectAsArrayBuffer(JSObject* obj, monkeycage::Tainted<size_t*> length,
                                        monkeycage::Tainted<uint8_t**> data) {
  return GetObjectAsArrayBuffer(obj, length.UNSAFE_unverified(),
                                data.UNSAFE_unverified());
}

inline void GetArrayBufferLengthAndData(JSObject* obj, monkeycage::Tainted<size_t*> length,
                                        monkeycage::Tainted<bool*> isSharedMemory, monkeycage::Tainted<uint8_t**> data) {
  GetArrayBufferLengthAndData(obj, length.UNSAFE_unverified(),
                              isSharedMemory.UNSAFE_unverified(),
                              data.UNSAFE_unverified());
}

inline uint8_t* GetArrayBufferData(JSObject* obj, monkeycage::Tainted<bool*> isSharedMemory,
                                   const JS::AutoRequireNoGC& nogc) {
  return GetArrayBufferData(obj, isSharedMemory.UNSAFE_unverified(), nogc);
}

inline bool HasDefinedArrayBufferDetachKey(JSContext* cx, JS::Handle<JSObject*> obj,
                                           monkeycage::Tainted<bool*> isDefined) {
  return HasDefinedArrayBufferDetachKey(cx, obj, isDefined.UNSAFE_unverified());
}

}
#endif
