/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Property descriptors and flags. */

#ifndef mc_PropertyDescriptor_h
#define mc_PropertyDescriptor_h

#include "js/PropertyDescriptor.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

inline bool JS_GetOwnPropertyDescriptorById(
    MCContext* cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {
  return JS_GetOwnPropertyDescriptorById(cx->cx_, obj, id, desc);
}

inline bool JS_GetOwnPropertyDescriptor(
    MCContext* cx, JS::Handle<JSObject*> obj, const char* name,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {
  return JS_GetOwnPropertyDescriptor(cx->cx_, obj, name, desc);
}

inline bool JS_GetOwnUCPropertyDescriptor(
    MCContext* cx, JS::Handle<JSObject*> obj, const char16_t* name,
    size_t namelen,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {
  return JS_GetOwnUCPropertyDescriptor(cx->cx_, obj, name, namelen, desc);
}

inline bool JS_GetPropertyDescriptorById(
    MCContext* cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc,
    JS::MutableHandle<JSObject*> holder) {
    return JS_GetPropertyDescriptorById(cx->cx_, obj, id, desc, holder);
}

inline bool JS_GetPropertyDescriptor(
    MCContext* cx, JS::Handle<JSObject*> obj, const char* name,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc,
    JS::MutableHandle<JSObject*> holder) {
    return JS_GetPropertyDescriptor(cx->cx_, obj, name, desc, holder);
}

inline bool JS_GetUCPropertyDescriptor(
    MCContext* cx, JS::Handle<JSObject*> obj, const char16_t* name,
    size_t namelen,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc,
    JS::MutableHandle<JSObject*> holder) {
    return JS_GetUCPropertyDescriptor(cx->cx_, obj, name, namelen, desc, holder);
}
#endif

#endif
