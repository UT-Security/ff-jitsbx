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
                                  JS::Handle<jsid> id, MC::SandboxCallback<JSNative> getter,
                                  MC::SandboxCallback<JSNative> setter, unsigned attrs) {
 return JS_DefinePropertyById(cx->cx_, obj, id, getter.UNSAFE_get(), setter.UNSAFE_get(), attrs);
}

inline bool JS_DefinePropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  JS::Handle<JSObject*> getter,
                                  JS::Handle<JSObject*> setter, unsigned attrs) {
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
                              const char* name, MC::SandboxCallback<JSNative> getter,
                              MC::SandboxCallback<JSNative> setter, unsigned attrs) {
 return JS_DefineProperty(cx->cx_, obj, name, getter.UNSAFE_get(), setter.UNSAFE_get(), attrs);
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
                              const char* name, uint32_t value, unsigned attrs) {
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
 return JS_DefineUCProperty(cx->cx_, obj, name, namelen, getter, setter, attrs);
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

inline bool JS_GetProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                           const char* name, MC::MutableHandle<JS::Value> vp) {
  return JS_GetProperty(cx->cx_, obj, name, vp.MC_INTERNAL_SAFE_get());
}

inline bool JS_SetPropertyById(MCContext* cx, JS::Handle<JSObject*> obj,
                               JS::Handle<jsid> id, JS::Handle<JS::Value> v) {
 return JS_SetPropertyById(cx->cx_, obj, id, v);
}

inline bool JS_SetProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                           const char* name, JS::Handle<JS::Value> v){
 return JS_SetProperty(cx->cx_, obj, name, v);
}

inline bool JS_SetUCProperty(MCContext* cx, JS::Handle<JSObject*> obj,
                             const char16_t* name, size_t namelen,
                             JS::Handle<JS::Value> v) {
 return JS_SetUCProperty(cx->cx_, obj, name, namelen, v);
}

inline bool JS_DefineFunctions(MCContext* cx, JS::Handle<JSObject*> obj,
                               const JSFunctionSpec* fs) {
  return JS_DefineFunctions(cx->cx_, obj, fs);
}

#endif

#endif
