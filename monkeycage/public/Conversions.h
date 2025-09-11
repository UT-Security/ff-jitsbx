/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* ECMAScript conversion operations. */

#ifndef mc_Conversions_h
#define mc_Conversions_h

#include "js/Conversions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline bool OrdinaryToPrimitive(MCContext* cx, HandleObject obj, JSType type,
                                MutableHandleValue vp) {
  return OrdinaryToPrimitive(cx->cx_, obj, type, vp);
}

inline bool ToNumber(MCContext* cx, HandleValue v, MC::Tainted<double*> out) {
  return ToNumber(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToInt32(MCContext* cx, JS::HandleValue v, MC::Tainted<int32_t*> out) {
  return ToInt32(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToUint32(MCContext* cx, HandleValue v, MC::Tainted<uint32_t*> out) {
  return ToUint32(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToInt16(MCContext* cx, JS::HandleValue v, MC::Tainted<int16_t*> out) {
  return ToInt16(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToUint16(MCContext* cx, HandleValue v, MC::Tainted<uint16_t*> out) {
  return ToUint16(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToInt8(MCContext* cx, JS::HandleValue v, MC::Tainted<int8_t*> out) {
  return ToInt8(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToUint8(MCContext* cx, JS::HandleValue v, MC::Tainted<uint8_t*> out) {
  return ToUint8(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToInt64(MCContext* cx, HandleValue v, MC::Tainted<int64_t*> out) {
  return ToInt64(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline bool ToUint64(MCContext* cx, HandleValue v, MC::Tainted<uint64_t*> out) {
  return ToUint64(cx->cx_, v, out.INTERNAL_unverified_safe());
}

inline JSString* ToString(MCContext* cx, JS::HandleValue v) {
  return ToString(cx->cx_, v);
}

inline JSObject* ToObject(MCContext* cx, HandleValue v) {
  return ToObject(cx->cx_, v);
}

}

#endif

#endif
