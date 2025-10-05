/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xpcprivate.h"
#include "XPCWrapper.h"
#include "WrapperFactory.h"
#include "AccessCheck.h"

#include "monkeycage/PropertyAndElement.h"  // JS_DefineFunction

using namespace xpc;
using namespace mozilla;
using namespace JS;

namespace XPCNativeWrapper {

static inline bool ThrowException(nsresult ex, MCContext* cx) {
  XPCThrower::Throw(ex, cx);

  return false;
}

static MC::Tainted<bool> UnwrapNW(MC::Tainted<JSContext*> t_cx, unsigned argc, MC::Tainted<Value*> t_vp) {
  MCContext* cx = t_cx.copy_and_verify_address(MC_VerifyContext);
  Value* vp = t_vp.UNSAFE_unverified();

  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (args.length() != 1) {
    return ThrowException(NS_ERROR_XPC_NOT_ENOUGH_ARGS, cx);
  }

  MC::RootedValue v(cx, args[0]);
  if (!v.isObject() || !mc::IsCrossCompartmentWrapper(&v.toObject()) ||
      !WrapperFactory::AllowWaiver(&v.toObject())) {
    args.rval().set(v);
    return true;
  }

  bool ok = xpc::WrapperFactory::WaiveXrayAndWrap(cx, &v);
  NS_ENSURE_TRUE(ok, false);
  args.rval().set(v);
  return true;
}

static MC::Tainted<bool> XrayWrapperConstructor(MC::Tainted<JSContext*> t_cx, unsigned argc, MC::Tainted<Value*> t_vp) {
  MCContext* cx = t_cx.copy_and_verify_address(MC_VerifyContext);
  Value* vp = t_vp.UNSAFE_unverified();

  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (args.length() == 0) {
    return ThrowException(NS_ERROR_XPC_NOT_ENOUGH_ARGS, cx);
  }

  if (!args[0].isObject()) {
    if (args.isConstructing()) {
      return ThrowException(NS_ERROR_XPC_BAD_CONVERT_JS, cx);
    }

    args.rval().set(args[0]);
    return true;
  }

  args.rval().setObject(*js::UncheckedUnwrap(&args[0].toObject()));
  return JS_WrapValue(cx, args.rval());
}
// static
bool AttachNewConstructorObject(MCContext* aCx,
                                JS::HandleObject aGlobalObject) {
  MC::SandboxStack<JSAutoRealm> ar(aCx, aGlobalObject);

  static auto XrayWrapperConstructorCb = MC::Sandbox::RegisterTaintedCallback(XrayWrapperConstructor);
  JSFunction* xpcnativewrapper = JS_DefineFunction(
      aCx, aGlobalObject, "XPCNativeWrapper", XrayWrapperConstructorCb, 1,
      JSPROP_READONLY | JSPROP_PERMANENT | JSFUN_CONSTRUCTOR);
  if (!xpcnativewrapper) {
    return false;
  }
  MC::RootedObject obj(aCx, JS_GetFunctionObject(xpcnativewrapper));

  static auto UnwrapNWCb = MC::Sandbox::RegisterTaintedCallback(UnwrapNW);
  return JS_DefineFunction(aCx, obj, "unwrap", UnwrapNWCb, 1,
                           JSPROP_READONLY | JSPROP_PERMANENT) != nullptr;
}

}  // namespace XPCNativeWrapper

namespace XPCWrapper {

JSObject* UnsafeUnwrapSecurityWrapper(JSObject* obj) {
  if (js::IsProxy(obj)) {
    return js::UncheckedUnwrap(obj);
  }

  return obj;
}

}  // namespace XPCWrapper
