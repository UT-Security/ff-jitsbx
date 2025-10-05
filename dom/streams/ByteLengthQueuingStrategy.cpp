/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/ByteLengthQueuingStrategy.h"
#include "mozilla/dom/FunctionBinding.h"
#include "mozilla/dom/QueuingStrategyBinding.h"
#include "nsCOMPtr.h"
#include "nsISupports.h"

#include "monkeycage/TypeDecls.h"
#include "monkeycage/PropertyAndElement.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_INHERITED(ByteLengthQueuingStrategy,
                                                BaseQueuingStrategy)
NS_IMPL_ADDREF_INHERITED(ByteLengthQueuingStrategy, BaseQueuingStrategy)
NS_IMPL_RELEASE_INHERITED(ByteLengthQueuingStrategy, BaseQueuingStrategy)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(ByteLengthQueuingStrategy)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
NS_INTERFACE_MAP_END_INHERITING(BaseQueuingStrategy)

/* static */
already_AddRefed<ByteLengthQueuingStrategy>
ByteLengthQueuingStrategy::Constructor(const GlobalObject& aGlobal,
                                       const QueuingStrategyInit& aInit) {
  RefPtr<ByteLengthQueuingStrategy> strategy = new ByteLengthQueuingStrategy(
      aGlobal.GetAsSupports(), aInit.mHighWaterMark);
  return strategy.forget();
}

JSObject* ByteLengthQueuingStrategy::WrapObject(
    MCContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return ByteLengthQueuingStrategy_Binding::Wrap(aCx, this, aGivenProto);
}

static MC::Tainted<bool> ByteLengthQueuingStrategySize(MC::Tainted<JSContext*> t_cx, unsigned argc,
                                          MC::Tainted<JS::Value*> t_vp) {
  MCContext* cx = t_cx.copy_and_verify_address(MC_VerifyContext);
  JS::Value* vp = t_vp.UNSAFE_unverified();
  // https://streams.spec.whatwg.org/#blqs-internal-slots
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  // Step 1: Return ? GetV(chunk, "byteLength").
  MC::Rooted<JSObject*> chunkObj(cx, JS::ToObject(cx, args.get(0)));
  if (!chunkObj) {
    return false;
  }

  return JS_GetProperty(cx, chunkObj, "byteLength", args.rval());
}

// https://streams.spec.whatwg.org/#blqs-size
already_AddRefed<Function> ByteLengthQueuingStrategy::GetSize(
    ErrorResult& aRv) {
  // Step 1. Return this's relevant global object's ByteLength queuing strategy
  // size function.
  if (RefPtr<Function> fun =
          mGlobal->GetByteLengthQueuingStrategySizeFunction()) {
    return fun.forget();
  }

  // Note: Instead of eagerly allocating a size function for every global object
  // we do it lazily once in this getter.
  // After this point the steps refer to:
  // https://streams.spec.whatwg.org/#byte-length-queuing-strategy-size-function

  AutoJSAPI jsapi;
  if (!jsapi.Init(mGlobal)) {
    aRv.ThrowUnknownError("Internal error");
    return nullptr;
  }
  MCContext* cx = jsapi.mcx();

  // Step 1. Let steps be the following steps, given chunk
  // Note: See ByteLengthQueuingStrategySize instead.

  // Step 2. Let F be !CreateBuiltinFunction(steps, 1, "size", « »,
  // globalObject’s relevant Realm).
  static auto ByteLengthQueuingStrategySizeCb =
      MC::Sandbox::RegisterTaintedCallback(ByteLengthQueuingStrategySize);
  MC::Rooted<JSFunction*> sizeFunction(
      cx, JS_NewFunction(cx, ByteLengthQueuingStrategySizeCb, 1, 0, "size"));
  if (!sizeFunction) {
    aRv.StealExceptionFromJSContext(cx);
    return nullptr;
  }

  // Step 3. Set globalObject’s byte length queuing strategy size function to
  // a Function that represents a reference to F,
  // with callback context equal to globalObject’s relevant settings object.
  MC::Rooted<JSObject*> funObj(cx, JS_GetFunctionObject(sizeFunction));
  MC::Rooted<JSObject*> global(cx, mGlobal->GetGlobalJSObject());
  RefPtr<Function> function = new Function(cx, funObj, global, mGlobal);
  mGlobal->SetByteLengthQueuingStrategySizeFunction(function);

  return function.forget();
}

}  // namespace mozilla::dom
