/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * JSON serialization and deserialization operations.
 */

#ifndef mc_JSON_h
#define mc_JSON_h

#include "js/JSON.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

inline bool JS_ParseJSON(MCContext* cx, const char16_t* chars, uint32_t len,
                         JS::MutableHandle<JS::Value> vp) {
  return JS_ParseJSON(cx->cx_, chars, len, vp);
}

inline bool JS_ParseJSON(MCContext* cx, JS::Handle<JSString*> str,
                                       JS::MutableHandle<JS::Value> vp) {
  return JS_ParseJSON(cx->cx_, str, vp);
}

inline bool JS_ParseJSON(MCContext* cx, const JS::Latin1Char* chars,
                         uint32_t len, JS::MutableHandle<JS::Value> vp) {
  return JS_ParseJSON(cx->cx_, chars, len, vp);
}
#endif

#endif
