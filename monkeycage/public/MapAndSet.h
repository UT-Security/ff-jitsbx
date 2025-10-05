/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Weak Maps.
 */

#ifndef mc_MapAndSet_h
#define mc_MapAndSet_h

#include "js/MapAndSet.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {

/*
 * Map
 */
inline JSObject* NewMapObject(MCContext* cx) { return NewMapObject(cx->cx_); }

inline uint32_t MapSize(MCContext* cx, HandleObject obj) {
  return MapSize(cx->cx_, obj);
}

inline bool MapGet(MCContext* cx, HandleObject obj, HandleValue key,
                   MutableHandleValue rval) {
  return MapGet(cx->cx_, obj, key, rval);
}

inline bool MapHas(MCContext* cx, HandleObject obj, HandleValue key,
                   MC::Tainted<bool*> rval) {
  return MapHas(cx->cx_, obj, key, rval.INTERNAL_unverified_safe());
}

inline bool MapSet(MCContext* cx, HandleObject obj, HandleValue key,
                   HandleValue val) {
  return MapSet(cx->cx_, obj, key, val);
}

inline bool MapDelete(MCContext* cx, HandleObject obj, HandleValue key,
                      MC::Tainted<bool*> rval) {
  return MapDelete(cx->cx_, obj, key, rval.INTERNAL_unverified_safe());
}

inline bool MapClear(MCContext* cx, HandleObject obj) {
  return MapClear(cx->cx_, obj);
}

inline bool MapKeys(MCContext* cx, HandleObject obj, MutableHandleValue rval) {
  return MapKeys(cx->cx_, obj, rval);
}

inline bool MapValues(MCContext* cx, HandleObject obj,
                      MutableHandleValue rval) {
  return MapValues(cx->cx_, obj, rval);
}

inline bool MapEntries(MCContext* cx, HandleObject obj,
                       MutableHandleValue rval) {
  return MapEntries(cx->cx_, obj, rval);
}

inline bool MapForEach(MCContext* cx, HandleObject obj, HandleValue callbackFn,
                       HandleValue thisVal) {
  return MapForEach(cx->cx_, obj, callbackFn, thisVal);
}


/*
 * Set
 */
inline JSObject* NewSetObject(MCContext* cx) {
  return NewSetObject(cx->cx_);
}

inline uint32_t SetSize(MCContext* cx, HandleObject obj) {
  return SetSize(cx->cx_, obj);
}

inline bool SetHas(MCContext* cx, HandleObject obj,
                                 HandleValue key, MC::Tainted<bool*> rval) {
  return SetHas(cx->cx_, obj, key, rval.INTERNAL_unverified_safe());
}

inline bool SetDelete(MCContext* cx, HandleObject obj,
                                    HandleValue key, MC::Tainted<bool*> rval) {
  return SetDelete(cx->cx_, obj, key, rval.INTERNAL_unverified_safe());
}

inline bool SetAdd(MCContext* cx, HandleObject obj,
                                 HandleValue key) {
  return SetAdd(cx->cx_, obj, key);
}

inline bool SetClear(MCContext* cx, HandleObject obj) {
  return SetClear(cx->cx_, obj);
}

inline bool SetKeys(MCContext* cx, HandleObject obj,
                                  MutableHandleValue rval) {
  return SetKeys(cx->cx_, obj, rval);
}

inline bool SetValues(MCContext* cx, HandleObject obj,
                                    MutableHandleValue rval) {
  return SetValues(cx->cx_, obj, rval);
}

inline bool SetEntries(MCContext* cx, HandleObject obj,
                                     MutableHandleValue rval) {
  return SetEntries(cx->cx_, obj, rval);
}

inline bool SetForEach(MCContext* cx, HandleObject obj,
                                     HandleValue callbackFn,
                                     HandleValue thisVal) {
  return SetForEach(cx->cx_, obj, callbackFn, thisVal);
}

}  // namespace JS

namespace MC {

inline JSObject* NewMapObject(MCContext* cx) { return JS::NewMapObject(cx); }

inline JSObject* NewSetObject(MCContext* cx) {
  return JS::NewSetObject(cx);
}

}

#endif

#endif
