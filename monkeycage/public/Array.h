/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Array-related operations. */

#ifndef mc_Array_h
#define mc_Array_h

#include "js/Array.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline bool IsArrayObject(JSContext* cx, Handle<Value> value,
                                        monkeycage::Tainted<bool*> isArray) {
  return JS::IsArrayObject(cx, value, isArray.UNSAFE_unverified());
}

inline bool IsArrayObject(JSContext* cx, Handle<JSObject*> obj,
                                        monkeycage::Tainted<bool*> isArray) {
  return JS::IsArrayObject(cx, obj, isArray.UNSAFE_unverified());
}

inline  bool GetArrayLength(JSContext* cx, Handle<JSObject*> obj,
                                         monkeycage::Tainted<uint32_t*> lengthp) {
  return JS::GetArrayLength(cx, obj, lengthp.UNSAFE_unverified());
}

inline bool IsArray(JSContext* cx, Handle<JSObject*> obj,
                                  monkeycage::Tainted<bool*> isArray) {
  return JS::IsArray(cx, obj, isArray.UNSAFE_unverified());
}

inline bool IsArray(JSContext* cx, Handle<JSObject*> obj,
                                  monkeycage::Tainted<IsArrayAnswer*> answer) {
  return JS::IsArray(cx, obj, answer.UNSAFE_unverified());
}

}

#endif
