/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Property and element API. */

#ifndef mc_PropertyAndElement_h
#define mc_PropertyAndElement_h

#include "js/PropertyAndElement.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/RootingAPI.h"

inline bool JS_GetProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                           const char* name, MC::MutableHandle<JS::Value> vp) {
 return JS_GetProperty(cx->cx_, obj, name, vp.MC_INTERNAL_SAFE_get());
}

inline bool JS_DefineFunctions(MCContext* cx, JS::Handle<JSObject*> obj,
                               const JSFunctionSpec* fs) {
  return JS_DefineFunctions(cx->cx_, obj, fs);
}

#endif

#endif
