/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Property and element API. */

#ifndef mc_PropertyAndElement_h
#define mc_PropertyAndElement_h

#include "SandboxCallback.h"
#include "js/PropertyAndElement.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/SandboxCallback.h"

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JS::PropertyDescriptor> desc,
                                  JS::ObjectOpResult& result) {
  return JS_DefinePropertyById(cx->cx_, obj, id, desc, result);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JS::PropertyDescriptor> desc) {
  return JS_DefinePropertyById(cx->cx_, obj, id, desc);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JS::Value> value, unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, value, attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  MC::SandboxCallback<JSNative> getter,
                                  MC::SandboxCallback<JSNative> setter,
                                  unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, getter.UNSAFE_get(),
                               setter.UNSAFE_get(), attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JSObject*> getter,
                                  JS::Handle<JSObject*> setter,
                                  unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, getter, setter, attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JSObject*> value, unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, value, attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JSString*> value, unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, value, attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id, int32_t value,
                                  unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, value, attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id, uint32_t value,
                                  unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, value, attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id, double value,
                                  unsigned attrs) {
  return JS_DefinePropertyById(cx->cx_, obj, id, value, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, JS::Handle<JS::Value> value,
                              unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, value, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name,
                              MC::SandboxCallback<JSNative> getter,
                              MC::SandboxCallback<JSNative> setter,
                              unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, getter.UNSAFE_get(),
                           setter.UNSAFE_get(), attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, JS::Handle<JSObject*> getter,
                              JS::Handle<JSObject*> setter, unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, getter, setter, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, JS::Handle<JSObject*> value,
                              unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, value, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, JS::Handle<JSString*> value,
                              unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, value, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, int32_t value, unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, value, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, uint32_t value,
                              unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, value, attrs);
}

inline bool JS_DefineProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, double value, unsigned attrs) {
  return JS_DefineProperty(cx->cx_, obj, name, value, attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::Handle<JS::PropertyDescriptor> desc,
                                JS::ObjectOpResult& result) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, desc, result);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::Handle<JS::PropertyDescriptor> desc) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, desc);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::Handle<JS::Value> value, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, value, attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::Handle<JSObject*> getter,
                                JS::Handle<JSObject*> setter, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, getter, setter,
                             attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::Handle<JSObject*> value, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, value, attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::Handle<JSString*> value, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, value, attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                int32_t value, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, value, attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                uint32_t value, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, value, attrs);
}

inline bool JS_DefineUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                double value, unsigned attrs) {
  return JS_DefineUCProperty(cx->cx_, obj, name, namelen, value, attrs);
}

inline bool JS_HasPropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                        JS::Handle<jsid> id, bool* foundp) {
  return JS_HasPropertyById(cx->cx_, obj, id, foundp);
}

inline bool JS_HasProperty(MCContext* cx, JS::Handle<JSObject*> obj, const char* name,
                    bool* foundp) {
  return JS_HasProperty(cx->cx_, obj, name, foundp);
}

inline bool JS_HasUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                      const char16_t* name, size_t namelen, bool* vp) {
  return JS_HasUCProperty(cx->cx_, obj, name, namelen, vp);
}

inline bool JS_HasElement(MCContext* cx, JS::Handle<JSObject*> obj, uint32_t index,
                   bool* foundp) {
  return JS_HasElement(cx->cx_, obj, index, foundp);
}

inline bool JS_HasOwnPropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id, bool* foundp) {
 return JS_HasOwnPropertyById(cx->cx_, obj, id, foundp);
}

inline bool JS_HasOwnProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, bool* foundp) {
 return JS_HasOwnProperty(cx->cx_, obj, name, foundp);
}

inline bool JS_ForwardGetPropertyTo(MCContext* cx, JS::Handle<JSObject*> obj,
                                    JS::Handle<jsid> id,
                                    JS::Handle<JS::Value> receiver,
                                    JS::MutableHandleValue vp) {
  return JS_ForwardGetPropertyTo(cx->cx_, obj, id, receiver, vp);
}

inline bool JS_ForwardGetElementTo(MCContext* cx, JS::Handle<JSObject*> obj,
                                   uint32_t index,
                                   JS::Handle<JSObject*> receiver,
                                   JS::MutableHandleValue vp) {
  return JS_ForwardGetElementTo(cx->cx_, obj, index, receiver, vp);
}

inline bool JS_GetPropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                               JS::Handle<jsid> id, JS::MutableHandleValue vp) {
 return JS_GetPropertyById(cx->cx_, obj, id, vp);
}

inline bool JS_GetProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                           const char* name, MC::MutableHandle<JS::Value> vp) {
  return JS_GetProperty(cx->cx_, obj, name, vp.MC_INTERNAL_SAFE_get());
}

inline bool JS_GetElement(MCContext* cx, JS::Handle<JSObject*> obj,
                          uint32_t index, JS::MutableHandleValue vp) {
  return JS_GetElement(cx->cx_, obj, index, vp);
}

inline bool JS_SetPropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                               JS::Handle<jsid> id, JS::Handle<JS::Value> v) {
  return JS_SetPropertyById(cx->cx_, obj, id, v);
}

inline bool JS_SetProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                           const char* name, JS::Handle<JS::Value> v) {
  return JS_SetProperty(cx->cx_, obj, name, v);
}

inline bool JS_SetUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                             const char16_t* name, size_t namelen,
                             JS::Handle<JS::Value> v) {
  return JS_SetUCProperty(cx->cx_, obj, name, namelen, v);
}

inline bool JS_DeletePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::ObjectOpResult& result) {
  return JS_DeletePropertyById(cx->cx_, obj, id, result);
}

inline bool JS_DeleteProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, JS::ObjectOpResult& result) {
  return JS_DeleteProperty(cx->cx_, obj, name, result);
}

inline bool JS_DeleteUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                JS::ObjectOpResult& result) {
  return JS_DeleteUCProperty(cx->cx_, obj, name, namelen, result);
}

inline bool JS_DeleteElement(MCContext* cx, JS::Handle<JSObject*> obj,
                             uint32_t index, JS::ObjectOpResult& result) {
  return JS_DeleteElement(cx->cx_, obj, index, result);
}

inline bool JS_DeletePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  jsid id) {
  return JS_DeletePropertyById(cx->cx_, obj, id);
}

inline bool JS_DeleteProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                              const char* name) {
  return JS_DeleteProperty(cx->cx_, obj, name);
}

inline bool JS_DeleteElement(MCContext* cx, JS::Handle<JSObject*> obj,
                             uint32_t index) {
  return JS_DeleteElement(cx->cx_, obj, index);
}

inline bool JS_Enumerate(MCContext* cx, JS::Handle<JSObject*> obj,
                         JS::MutableHandle<JS::IdVector> props) {
 return JS_Enumerate(cx->cx_, obj, props);
}

//TODO(abhishek): Potentially need to change the JSPropertySpec args type.
inline bool JS_DefineProperties(MCContext* cx, JS::Handle<JSObject*> obj,
                                const JSPropertySpec* ps) {
 return JS_DefineProperties(cx->cx_, obj, ps);
}

inline bool JS_AlreadyHasOwnPropertyById(MCContext* cx,
                                         JS::Handle<JSObject*> obj,
                                         JS::Handle<jsid> id, bool* foundp) {
  return JS_AlreadyHasOwnPropertyById(cx->cx_, obj, id, foundp);
}

inline bool JS_AlreadyHasOwnProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                     const char* name, bool* foundp) {
  return JS_AlreadyHasOwnProperty(cx->cx_, obj, name, foundp);
}

inline bool JS_AlreadyHasOwnUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                                       const char16_t* name, size_t namelen,
                                       bool* foundp) {
  return JS_AlreadyHasOwnUCProperty(cx->cx_, obj, name, namelen, foundp);
}

inline bool JS_AlreadyHasOwnElement(MCContext* cx, JS::Handle<JSObject*> obj,
                                    uint32_t index, bool* foundp) {
  return JS_AlreadyHasOwnElement(cx->cx_, obj, index, foundp);
}

inline bool JS_DefineFunctions(MCContext* cx, JS::Handle<JSObject*> obj,
                               const JSFunctionSpec* fs) {
  return JS_DefineFunctions(cx->cx_, obj, fs);
}

inline JSFunction* JS_DefineFunction(MCContext* cx, JS::Handle<JSObject*> obj,
                                     const char* name,
                                     MC::SandboxCallback<JSNative> call,
                                     unsigned nargs, unsigned attrs) {
  return JS_DefineFunction(cx->cx_, obj, name, call.UNSAFE_get(), nargs, attrs);
}

inline JSFunction* JS_DefineUCFunction(MCContext* cx, JS::Handle<JSObject*> obj,
                                       const char16_t* name, size_t namelen,
                                       MC::SandboxCallback<JSNative> call,
                                       unsigned nargs, unsigned attrs) {
  return JS_DefineUCFunction(cx->cx_, obj, name, namelen, call.UNSAFE_get(),
                             nargs, attrs);
}

inline JSFunction* JS_DefineFunctionById(MCContext* cx,
                                         JS::Handle<JSObject*> obj,
                                         JS::Handle<jsid> id,
                                         MC::SandboxCallback<JSNative> call,
                                         unsigned nargs, unsigned attrs) {
  return JS_DefineFunctionById(cx->cx_, obj, id, call.UNSAFE_get(), nargs,
                               attrs);
}

#endif

#endif
