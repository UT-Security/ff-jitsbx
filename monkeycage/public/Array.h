/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Array-related operations. */

#ifndef mc_Array_h
#define mc_Array_h

#include "js/Array.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline JSObject* NewArrayObject(MCContext* cx, const HandleValueArray& contents) {
 return NewArrayObject(cx->cx_, contents);
}

inline JSObject* NewArrayObject(MCContext* cx, size_t length) {
 return NewArrayObject(cx->cx_, length);
}

inline bool IsArrayObject(MCContext* cx, Handle<Value> value,
                          MC::Tainted<bool*> isArray) {
  return IsArrayObject(cx->cx_, value, isArray.INTERNAL_unverified_safe());
}

inline bool IsArrayObject(MCContext* cx, Handle<JSObject*> obj,
                          MC::Tainted<bool*> isArray) {
  return IsArrayObject(cx->cx_, obj, isArray.INTERNAL_unverified_safe());
}

inline bool GetArrayLength(MCContext* cx, Handle<JSObject*> obj,
                           MC::Tainted<uint32_t*> lengthp) {
  return GetArrayLength(cx->cx_, obj, lengthp.INTERNAL_unverified_safe());
}

inline bool SetArrayLength(MCContext* cx, Handle<JSObject*> obj,
                           uint32_t length) {
  return SetArrayLength(cx->cx_, obj, length);
}
}  // namespace JS

#endif

#endif
