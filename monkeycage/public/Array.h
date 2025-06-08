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

inline bool GetArrayLength(MCContext* cx, Handle<JSObject*> obj,
                           MC::Tainted<uint32_t*> lengthp) {
  return GetArrayLength(cx->cx_, obj, lengthp.UNSAFE_unverified());
}

//TODO(abhishek): REMOVE
inline bool GetArrayLength(MCContext* cx, Handle<JSObject*> obj,
                           uint32_t* lengthp) {
  return GetArrayLength(cx->cx_, obj, lengthp);
}

inline bool SetArrayLength(MCContext* cx, Handle<JSObject*> obj,
                           uint32_t length) {
  return SetArrayLength(cx->cx_, obj, length);
}
}  // namespace JS

#endif

#endif
