/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Equality operations. */

#ifndef mc_Equality_h
#define mc_Equality_h

#include "js/Equality.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline bool StrictlyEqual(MCContext* cx, JS::Handle<JS::Value> v1,
                   JS::Handle<JS::Value> v2, MC::Tainted<bool*> equal) {
  return StrictlyEqual(cx->cx_, v1, v2, equal.INTERNAL_unverified_safe());
}

inline bool LooselyEqual(MCContext* cx, JS::Handle<JS::Value> v1,
                                       JS::Handle<JS::Value> v2, MC::Tainted<bool*> equal) {
  return LooselyEqual(cx->cx_, v1, v2, equal.INTERNAL_unverified_safe());
}

inline bool SameValue(MCContext* cx, JS::Handle<JS::Value> v1,
                                    JS::Handle<JS::Value> v2, MC::Tainted<bool*> same) {
  return SameValue(cx->cx_, v1, v2, same.INTERNAL_unverified_safe());
}

}  // namespace JS

#endif

#endif
