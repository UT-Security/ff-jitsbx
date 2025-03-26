/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _xpc_WRAPPERFACTORY_H
#define _xpc_WRAPPERFACTORY_H

#include "js/Wrapper.h"
#include "monkeycage/Tainted.h"

namespace xpc {

class WrapperFactory {
 public:
  enum {
    WAIVE_XRAY_WRAPPER_FLAG = js::Wrapper::LAST_USED_FLAG << 1,
    IS_XRAY_WRAPPER_FLAG = WAIVE_XRAY_WRAPPER_FLAG << 1
  };

  // Return true if any of any of the nested wrappers have the flag set.
  static bool HasWrapperFlag(JSObject* wrapper, unsigned flag) {
    monkeycage::AutoStackTainted<unsigned> flags{0};
    js::UncheckedUnwrap(wrapper, true, flags.UNSAFE_unverified());
    return !!(*flags.UNSAFE_unverified() & flag);
  }

  static bool IsXrayWrapper(JSObject* wrapper) {
    return HasWrapperFlag(wrapper, IS_XRAY_WRAPPER_FLAG);
  }

  static bool IsCrossOriginWrapper(JSObject* obj);
//    return (js::IsProxy(obj) &&
//            js::GetProxyHandler(obj) == CrossOriginObjectWrapper::singleton());
//  }

  static bool IsOpaqueWrapper(JSObject* obj);

  static bool HasWaiveXrayFlag(JSObject* wrapper) {
    return HasWrapperFlag(wrapper, WAIVE_XRAY_WRAPPER_FLAG);
  }

  static bool IsCOW(JSObject* wrapper);

  static JSObject* GetXrayWaiver(JS::Handle<JSObject*> obj);
  // If allowExisting is true, there is an existing waiver for obj in
  // its scope, but we want to replace it with the new one.
  static JSObject* CreateXrayWaiver(JSContext* cx, JS::Handle<JSObject*> obj,
                                    bool allowExisting = false);
  static JSObject* WaiveXray(JSContext* cx, JSObject* obj);

  // Computes whether we should allow the creation of an Xray waiver from
  // |target| to |origin|.
  static bool AllowWaiver(JS::Compartment* target, JS::Compartment* origin);

  // Convenience method for the above, operating on a wrapper.
  static bool AllowWaiver(JSObject* wrapper);

  // Prepare a given object for wrapping in a new compartment.
  static void PrepareForWrapping(JSContext* cx, JS::Handle<JSObject*> scope,
                                 JS::Handle<JSObject*> origObj,
                                 JS::Handle<JSObject*> obj,
                                 JS::Handle<JSObject*> objectPassedToWrap,
                                 JS::MutableHandle<JSObject*> retObj);

  // Rewrap an object that is about to cross compartment boundaries.
  static JSObject* Rewrap(JSContext* cx, JS::Handle<JSObject*> existing,
                          JS::Handle<JSObject*> obj);

  // Wrap wrapped object into a waiver wrapper and then re-wrap it.
  static bool WaiveXrayAndWrap(JSContext* cx, JS::MutableHandle<JS::Value> vp);
  static bool WaiveXrayAndWrap(JSContext* cx,
                               JS::MutableHandle<JSObject*> object);
};

}  // namespace xpc

#endif /* _xpc_WRAPPERFACTORY_H */
