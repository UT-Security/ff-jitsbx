/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Realm_h
#define mc_Realm_h

#include "js/Realm.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

namespace JS {

inline Realm* GetCurrentRealmOrNull(MCContext* cx) {
  return GetCurrentRealmOrNull(cx->cx_);
}

inline void SetDestroyRealmCallback(
    MCContext* cx, MC::SandboxCallback<DestroyRealmCallback> callback) {
  return SetDestroyRealmCallback(cx->cx_, callback.UNSAFE_get());
}

inline void SetRealmNameCallback(
    MCContext* cx, MC::SandboxCallback<RealmNameCallback> callback) {
  return SetRealmNameCallback(cx->cx_, callback.UNSAFE_get());
}

inline JSObject* GetRealmObjectPrototype(MCContext* cx) {
  return GetRealmObjectPrototype(cx->cx_);
}

inline JS::Handle<JSObject*> GetRealmObjectPrototypeHandle(MCContext* cx) {
  return GetRealmObjectPrototypeHandle(cx->cx_);
}

inline JSObject* GetRealmFunctionPrototype(MCContext* cx) {
  return GetRealmFunctionPrototype(cx->cx_);
}

inline JSObject* GetRealmArrayPrototype(MCContext* cx) {
  return GetRealmArrayPrototype(cx->cx_);
}

inline JSObject* GetRealmErrorPrototype(MCContext* cx) {
  return GetRealmErrorPrototype(cx->cx_);
}

inline JSObject* GetRealmIteratorPrototype(MCContext* cx) {
  return GetRealmIteratorPrototype(cx->cx_);
}

inline JSObject* GetRealmAsyncIteratorPrototype(MCContext* cx) {
  return GetRealmAsyncIteratorPrototype(cx->cx_);
}

inline JSObject* GetRealmKeyObject(MCContext* cx) {
  return GetRealmKeyObject(cx->cx_);
}

inline Realm* GetFunctionRealm(MCContext* cx, HandleObject objArg) {
  return GetFunctionRealm(cx->cx_, objArg);
}

inline JS::Realm* EnterRealm(MCContext* cx, JSObject* target) {
  return EnterRealm(cx->cx_, target);
}

inline void LeaveRealm(MCContext* cx, JS::Realm* oldRealm) {
  return LeaveRealm(cx->cx_, oldRealm);  
}

}
#endif

#endif
