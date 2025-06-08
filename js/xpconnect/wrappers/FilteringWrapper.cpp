/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FilteringWrapper.h"
#include "AccessCheck.h"
#include "ChromeObjectWrapper.h"
#include "XrayWrapper.h"
#include "nsJSUtils.h"
#include "mozilla/ErrorResult.h"
#include "xpcpublic.h"
#include "xpcprivate.h"

#include "mcapi.h"
#include "js/Symbol.h"

using namespace JS;
using namespace js;

namespace xpc {

static JS::SymbolCode sCrossOriginWhitelistedSymbolCodes[] = {
    JS::SymbolCode::toStringTag, JS::SymbolCode::hasInstance,
    JS::SymbolCode::isConcatSpreadable};

static bool IsCrossOriginWhitelistedSymbol(JSContext* cx, JS::HandleId id) {
  if (!id.isSymbol()) {
    return false;
  }

  JS::Symbol* symbol = id.toSymbol();
  for (auto code : sCrossOriginWhitelistedSymbolCodes) {
    if (symbol == JS::GetWellKnownSymbol(cx, code)) {
      return true;
    }
  }

  return false;
}

bool IsCrossOriginWhitelistedProp(JSContext* cx, JS::HandleId id) {
  return id == GetJSIDByIndex(cx, XPCJSContext::IDX_THEN) ||
         IsCrossOriginWhitelistedSymbol(cx, id);
}

bool AppendCrossOriginWhitelistedPropNames(JSContext* cx,
                                           JS::MutableHandleIdVector props) {
  // Add "then" if it's not already in the list.
  MC::RootedIdVector thenProp(cx);
  if (!thenProp.append(GetJSIDByIndex(cx, XPCJSContext::IDX_THEN))) {
    return false;
  }

  if (!AppendUnique(cx, props, thenProp)) {
    return false;
  }

  // Now add the three symbol-named props cross-origin objects have.
#ifdef DEBUG
  for (size_t n = 0; n < props.length(); ++n) {
    MOZ_ASSERT(!props[n].isSymbol(), "Unexpected existing symbol-name prop");
  }
#endif
  if (!props.reserve(
          props.length() +
          mozilla::ArrayLength(sCrossOriginWhitelistedSymbolCodes))) {
    return false;
  }

  for (auto code : sCrossOriginWhitelistedSymbolCodes) {
    props.infallibleAppend(JS::GetWellKnownSymbolKey(cx, code));
  }

  return true;
}

// Note: Previously, FilteringWrapper supported complex access policies where
// certain properties on an object were accessible and others weren't. Today,
// the only supported policies are Opaque and OpaqueWithCall, none of which need
// that. So we just stub out the unreachable paths.
template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::getOwnPropertyDescriptor(
    MCContext* cx, HandleObject wrapper, HandleId id,
    MutableHandle<mozilla::Maybe<PropertyDescriptor>> desc) const {
  MOZ_CRASH("FilteringWrappers are now always opaque");
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::ownPropertyKeys(
    MCContext* cx, HandleObject wrapper, MutableHandleIdVector props) const {
  MOZ_CRASH("FilteringWrappers are now always opaque");
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::getOwnEnumerablePropertyKeys(
    MCContext* cx, HandleObject wrapper, MutableHandleIdVector props) const {
  MOZ_CRASH("FilteringWrappers are now always opaque");
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::enumerate(
    MCContext* cx, HandleObject wrapper,
    JS::MutableHandleIdVector props) const {
  MOZ_CRASH("FilteringWrappers are now always opaque");
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::call(MCContext* cx,
                                          JS::Handle<JSObject*> wrapper,
                                          const JS::CallArgs& args) const {
  if (!Policy::checkCall(cx, wrapper, args)) {
    return false;
  }
  return Base::call(cx, wrapper, args);
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::construct(MCContext* cx,
                                               JS::Handle<JSObject*> wrapper,
                                               const JS::CallArgs& args) const {
  if (!Policy::checkCall(cx, wrapper, args)) {
    return false;
  }
  return Base::construct(cx, wrapper, args);
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::nativeCall(
    MCContext* cx, JS::IsAcceptableThis test, JS::NativeImpl impl,
    const JS::CallArgs& args) const {
  if (Policy::allowNativeCall(cx, test, impl)) {
    return Base::Permissive::nativeCall(cx, test, impl, args);
  }
  return Base::Restrictive::nativeCall(cx, test, impl, args);
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::getPrototype(
    MCContext* cx, JS::HandleObject wrapper,
    JS::MutableHandleObject protop) const {
  // Filtering wrappers do not allow access to the prototype.
  protop.set(nullptr);
  return true;
}

template <typename Base, typename Policy>
bool FilteringWrapper<Base, Policy>::enter(MCContext* cx, HandleObject wrapper,
                                           HandleId id, Wrapper::Action act,
                                           bool mayThrow, MC::Tainted<bool*> bp) const {
  if (!Policy::check(cx, wrapper, id, act)) {
    *bp =
        JS_IsExceptionPending(cx) ? false : Policy::deny(cx, act, id, mayThrow);
    return false;
  }
  *bp = true;
  return true;
}

#define NNXOW FilteringWrapper<mc::CrossCompartmentSecurityWrapper, Opaque>
#define NNXOWC FilteringWrapper<mc::CrossCompartmentSecurityWrapper, OpaqueWithCall>

template <>
const NNXOW* NNXOW::getSingleton() {
  static const NNXOW inner_(0);
  return &inner_;
}
template <>
const NNXOWC* NNXOWC::getSingleton() {
  static const NNXOWC inner_(0);
  return &inner_;
}

template class NNXOW;
template class NNXOWC;
template class ChromeObjectWrapperBase;
}  // namespace xpc
