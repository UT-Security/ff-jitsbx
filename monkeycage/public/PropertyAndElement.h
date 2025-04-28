/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Property and element API. */

#ifndef mc_PropertyAndElement_h
#define mc_PropertyAndElement_h

#include "js/PropertyAndElement.h"
#include "monkeycage/Tainted.h"

inline bool JS_DefineUCProperty(
    JSContext* cx, JS::Handle<JSObject*> obj, const char16_t* name,
    size_t namelen, JS::Handle<JS::PropertyDescriptor> desc,
    monkeycage::Tainted<JS::ObjectOpResult*> result) {
  return JS_DefineUCProperty(cx, obj, name, namelen, desc, *result.UNSAFE_unverified());
}

inline bool JS_HasPropertyById(JSContext* cx, JS::Handle<JSObject*> obj,
                               JS::Handle<jsid> id, monkeycage::Tainted<bool*> foundp) {
  return JS_HasPropertyById(cx, obj, id, foundp.UNSAFE_unverified());
}

inline bool JS_HasProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                           const char* name, monkeycage::Tainted<bool*> foundp) {
  return JS_HasProperty(cx, obj, name, foundp.UNSAFE_unverified());
}

inline bool JS_HasUCProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                             const char16_t* name, size_t namelen, monkeycage::Tainted<bool*> vp) {
  return JS_HasUCProperty(cx, obj, name, namelen, vp.UNSAFE_unverified());
}

inline bool JS_HasElement(JSContext* cx, JS::Handle<JSObject*> obj,
                          uint32_t index, monkeycage::Tainted<bool*> foundp) {
  return JS_HasElement(cx, obj, index, foundp.UNSAFE_unverified());
}

inline bool JS_HasOwnPropertyById(JSContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id, monkeycage::Tainted<bool*> foundp) {
  return JS_HasOwnPropertyById(cx, obj, id, foundp.UNSAFE_unverified());
}

inline bool JS_HasOwnProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, monkeycage::Tainted<bool*> foundp) {
  return JS_HasOwnProperty(cx, obj, name, foundp.UNSAFE_unverified());
}

inline bool JS_ForwardSetPropertyTo(
    JSContext* cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id,
    JS::Handle<JS::Value> v, JS::Handle<JS::Value> receiver,
    monkeycage::Tainted<JS::ObjectOpResult*> result) {
  return false;
}

inline bool JS_DeletePropertyById(JSContext* cx, JS::Handle<JSObject*> obj,
                                  JS::Handle<jsid> id,
                                  monkeycage::Tainted<JS::ObjectOpResult*> result) {
  return JS_DeletePropertyById(cx, obj, id, *result.UNSAFE_unverified());
}

inline bool JS_DeleteProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                              const char* name, monkeycage::Tainted<JS::ObjectOpResult*> result) {
  return JS_DeleteProperty(cx, obj, name, *result.UNSAFE_unverified());
}

inline bool JS_DeleteUCProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                                const char16_t* name, size_t namelen,
                                monkeycage::Tainted<JS::ObjectOpResult*> result) {
  return JS_DeleteUCProperty(cx, obj, name, namelen, *result.UNSAFE_unverified());
}

inline bool JS_DeleteElement(JSContext* cx, JS::Handle<JSObject*> obj,
                             uint32_t index, monkeycage::Tainted<JS::ObjectOpResult*> result) {
  return JS_DeleteElement(cx, obj, index, *result.UNSAFE_unverified());
}

inline bool JS_AlreadyHasOwnPropertyById(JSContext* cx,
                                         JS::Handle<JSObject*> obj,
                                         JS::Handle<jsid> id,
                                         monkeycage::Tainted<bool*> foundp) {
  return JS_AlreadyHasOwnPropertyById(cx, obj, id, foundp.UNSAFE_unverified());
}

inline bool JS_AlreadyHasOwnProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                                     const char* name,
                                     monkeycage::Tainted<bool*> foundp) {
  return JS_AlreadyHasOwnProperty(cx, obj, name, foundp.UNSAFE_unverified());
}

inline bool JS_AlreadyHasOwnUCProperty(JSContext* cx, JS::Handle<JSObject*> obj,
                                       const char16_t* name, size_t namelen,
                                       monkeycage::Tainted<bool*> foundp) {
  return JS_AlreadyHasOwnUCProperty(cx, obj, name, namelen, foundp.UNSAFE_unverified());
}

inline bool JS_AlreadyHasOwnElement(JSContext* cx, JS::Handle<JSObject*> obj,
                                    uint32_t index,
                                    monkeycage::Tainted<bool*> foundp) {
  return JS_AlreadyHasOwnElement(cx, obj, index, foundp.UNSAFE_unverified());
}
#endif
