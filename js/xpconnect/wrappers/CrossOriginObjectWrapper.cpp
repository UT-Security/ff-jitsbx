/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CrossOriginObjectWrapper.h"
#include "mozilla/dom/MaybeCrossOriginObject.h"
#include "js/Object.h"              // JS::GetPrivate, JS::GetCompartment

using namespace js;
using namespace JS;
using namespace mozilla;

namespace xpc {
const CrossOriginObjectWrapper* CrossOriginObjectWrapper::singleton() {
  static const CrossOriginObjectWrapper s;

  return &s;
}

bool CrossOriginObjectWrapper::dynamicCheckedUnwrapAllowed(
    HandleObject obj, JSContext* cx) const {
  MOZ_ASSERT(js::GetProxyHandler(obj) == js::sandbox::GetProxyHandler(this),
             "Why are we getting called for some random object?");
  JSObject* target = wrappedObject(obj);
  return dom::MaybeCrossOriginObjectMixins::IsPlatformObjectSameOrigin(cx,
                                                                       target);
}
}
