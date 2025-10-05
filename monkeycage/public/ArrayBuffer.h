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

inline JSObject* NewArrayBuffer(MCContext* cx, size_t nbytes) {
  return NewArrayBuffer(cx->cx_, nbytes);
}

//TODO(JS_SANDBOX): Unsafe overload
inline JSObject* NewArrayBufferWithContents(MCContext* cx, size_t nbytes,
                                            void* contents) {
  return NewArrayBufferWithContents(cx->cx_, nbytes, contents);
}

inline JSObject* NewArrayBufferWithContents(MCContext* cx, size_t nbytes,
                                            MC::Tainted<void*> contents) {
  return NewArrayBufferWithContents(cx->cx_, nbytes,
                                    contents.UNSAFE_unverified());
}

inline JSObject* CopyArrayBuffer(
    MCContext* cx, JS::Handle<JSObject*> maybeArrayBuffer) {
 return CopyArrayBuffer(cx->cx_, maybeArrayBuffer);
}

inline void GetArrayBufferLengthAndData(JSObject* obj,
                                        MC::Tainted<size_t*> length,
                                        MC::Tainted<bool*> isSharedMemory,
                                        MC::Tainted<uint8_t**> data) {
  GetArrayBufferLengthAndData(obj, length.INTERNAL_unverified_safe(),
                              isSharedMemory.INTERNAL_unverified_safe(),
                              data.INTERNAL_unverified_safe());
}

inline MC::Tainted<uint8_t*> GetArrayBufferData(
    JSObject* obj, MC::Tainted<bool*> isSharedMemory,
    MC::Tainted<const AutoCheckCannotGC*> nogc) {
  MC::Tainted<uint8_t*> ret;
  ret.assign_raw_pointer(
      GetArrayBufferData(obj, isSharedMemory.INTERNAL_unverified_safe(),
                         *nogc.INTERNAL_unverified_safe()));
  return ret;
}

inline bool DetachArrayBuffer(MCContext* cx, Handle<JSObject*> obj) {
  return DetachArrayBuffer(cx->cx_, obj);
}

inline bool HasDefinedArrayBufferDetachKey(MCContext* cx, Handle<JSObject*> obj,
                                           MC::Tainted<bool*> isDefined) {
  return HasDefinedArrayBufferDetachKey(cx->cx_, obj,
                                        isDefined.INTERNAL_unverified_safe());
}

inline void* StealArrayBufferContents(MCContext* cx, Handle<JSObject*> obj) {
  return StealArrayBufferContents(cx->cx_, obj);
}

inline bool ArrayBufferCopyData(MCContext* cx, Handle<JSObject*> toBlock,
                                size_t toIndex, Handle<JSObject*> fromBlock,
                                size_t fromIndex, size_t count) {
  return ArrayBufferCopyData(cx->cx_, toBlock, toIndex, fromBlock, fromIndex,
                             count);
}

inline JSObject* ArrayBufferClone(MCContext* cx, Handle<JSObject*> srcBuffer,
                                  size_t srcByteOffset, size_t srcLength) {
  return ArrayBufferClone(cx->cx_, srcBuffer, srcByteOffset, srcLength);
}

}  // namespace JS
#endif

#endif
