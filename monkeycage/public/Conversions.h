/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* ECMAScript conversion operations. */

#ifndef mc_Conversions_h
#define mc_Conversions_h

#include "js/Conversions.h"
#include "monkeycage/Tainted.h"

namespace JS {

MOZ_ALWAYS_INLINE bool ToNumber(JSContext* cx, HandleValue v, monkeycage::Tainted<double*> out) {
  return ToNumber(cx, v, out.UNSAFE_unverified());
}
  
MOZ_ALWAYS_INLINE bool ToInt32(JSContext* cx, JS::HandleValue v, monkeycage::Tainted<int32_t*> out) {
  return ToInt32(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToUint32(JSContext* cx, HandleValue v, monkeycage::Tainted<uint32_t*> out) {
  return ToUint32(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToInt16(JSContext* cx, JS::HandleValue v, monkeycage::Tainted<int16_t*> out) {
  return ToInt16(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToUint16(JSContext* cx, HandleValue v, monkeycage::Tainted<uint16_t*> out) {
  return ToUint16(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToInt8(JSContext* cx, JS::HandleValue v, monkeycage::Tainted<int8_t*> out) {
  return ToInt8(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToUint8(JSContext* cx, JS::HandleValue v, monkeycage::Tainted<uint8_t*> out) {
  return ToUint8(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToInt64(JSContext* cx, HandleValue v, monkeycage::Tainted<int64_t*> out) {
  return ToInt64(cx, v, out.UNSAFE_unverified());
}

MOZ_ALWAYS_INLINE bool ToUint64(JSContext* cx, HandleValue v, monkeycage::Tainted<uint64_t*> out) {
  return ToUint64(cx, v, out.UNSAFE_unverified());
}

}

#endif
