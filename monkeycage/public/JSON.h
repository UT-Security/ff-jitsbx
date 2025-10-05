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
#include "monkeycage/SandboxCallback.h"

inline bool JS_Stringify(MCContext* cx, JS::MutableHandle<JS::Value> value,
                         JS::Handle<JSObject*> replacer,
                         JS::Handle<JS::Value> space,
                         MC::SandboxCallback<JSONWriteCallback> callback,
                         void* data) {
  return JS_Stringify(cx->cx_, value, replacer, space, callback.UNSAFE_get(),
                      data);
}

namespace JS {

inline bool ToJSONMaybeSafely(MCContext* cx, JS::Handle<JSObject*> input,
                              MC::SandboxCallback<JSONWriteCallback> callback,
                              void* data) {
  return ToJSONMaybeSafely(cx->cx_, input, callback.UNSAFE_get(), data);
}

inline bool ToJSON(MCContext* cx, Handle<Value> value,
                   Handle<JSObject*> replacer, Handle<Value> space,
                   MC::SandboxCallback<JSONWriteCallback> callback,
                   void* data) {
  return ToJSON(cx->cx_, value, replacer, space, callback.UNSAFE_get(), data);
}
}  // namespace JS

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
