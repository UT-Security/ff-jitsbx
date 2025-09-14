/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Weak Maps.
 */

#ifndef mc_WeakMap_h
#define mc_WeakMap_h

#include "js/WeakMap.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {

inline JSObject* NewWeakMapObject(MCContext* cx) {
  return NewWeakMapObject(cx->cx_);
}

inline bool GetWeakMapEntry(MCContext* cx, JS::HandleObject mapObj,
                            JS::HandleObject key, JS::MutableHandleValue val) {
  return GetWeakMapEntry(cx->cx_, mapObj, key, val);
}

inline bool SetWeakMapEntry(MCContext* cx, JS::HandleObject mapObj,
                            JS::HandleObject key, JS::HandleValue val) {
  return SetWeakMapEntry(cx->cx_, mapObj, key, val);
}

}  // namespace JS

#endif

#endif
