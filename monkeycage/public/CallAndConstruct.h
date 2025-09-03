/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Call and construct API. */

#ifndef mc_CallAndConstruct_h
#define mc_CallAndConstruct_h

#include "js/CallAndConstruct.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/RootingAPI.h"

inline bool JS_CallFunctionValue(
    MCContext* cx, JS::Handle<JSObject*> obj, JS::Handle<JS::Value> fval,
    const JS::HandleValueArray& args, JS::MutableHandle<JS::Value> rval) {
 return JS_CallFunctionValue(cx->cx_, obj, fval, args, rval);
}

inline bool JS_CallFunction(MCContext* cx, JS::Handle<JSObject*> obj,
                            JS::Handle<JSFunction*> fun,
                            const JS::HandleValueArray& args,
                            JS::MutableHandle<JS::Value> rval) {
 return JS_CallFunction(cx->cx_, obj, fun, args, rval);
}

inline bool JS_CallFunctionName(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char* name,
                                const JS::HandleValueArray& args,
                                MC::MutableHandle<JS::Value> rval) {
  return JS_CallFunctionName(cx->cx_, obj, name, args, rval.MC_INTERNAL_SAFE_get());
}

namespace JS {

inline bool Call(MCContext* cx, Handle<JSObject*> thisObj,
                 Handle<JSFunction*> fun, const HandleValueArray& args,
                 MutableHandle<Value> rval) {
  return Call(cx->cx_, thisObj, fun, args, rval);
}

inline bool Call(MCContext* cx, Handle<JSObject*> thisObj, Handle<Value> fun,
                 const HandleValueArray& args, MutableHandle<Value> rval) {
  return Call(cx->cx_, thisObj, fun, args, rval);
}

inline bool Call(MCContext* cx, Handle<JSObject*> thisObj, const char* name,
                 const HandleValueArray& args, MutableHandle<Value> rval) {
  return Call(cx->cx_, thisObj, name, args, rval);
}

inline bool Call(MCContext* cx, Handle<Value> thisv, Handle<Value> fun,
                 const HandleValueArray& args, MutableHandle<Value> rval) {
  return Call(cx->cx_, thisv, fun, args, rval);
}

inline bool Call(MCContext* cx, Handle<Value> thisv, Handle<JSObject*> funObj,
                 const HandleValueArray& args, MutableHandle<Value> rval) {
  return Call(cx->cx_, thisv, funObj, args, rval);
}

inline bool Construct(MCContext* cx, Handle<Value> fun,
                      Handle<JSObject*> newTarget, const HandleValueArray& args,
                      MutableHandle<JSObject*> objp) {
  return Construct(cx->cx_, fun, newTarget, args, objp);
}

inline bool Construct(MCContext* cx, Handle<Value> fun,
                      const HandleValueArray& args,
                      MutableHandle<JSObject*> objp) {
  return Construct(cx->cx_, fun, args, objp);
}
}  // namespace JS
#endif

#endif
