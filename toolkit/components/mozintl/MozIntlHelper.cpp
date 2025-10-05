/* -*- Mode: C++; tab-width: 2; indent-tabs-mode:nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "MozIntlHelper.h"
#include "mcapi.h"
#include "monkeycage/experimental/Intl.h"   // JS::AddMozDateTimeFormatConstructor
#include "monkeycage/PropertyAndElement.h"  // JS_DefineFunctions
#include "js/PropertySpec.h"
#include "monkeycage/Wrapper.h"

using namespace mozilla;

NS_IMPL_ISUPPORTS(MozIntlHelper, mozIMozIntlHelper)

MozIntlHelper::MozIntlHelper() = default;

MozIntlHelper::~MozIntlHelper() = default;

static nsresult AddFunctions(MCContext* cx, JS::Handle<JS::Value> val,
                             const JSFunctionSpec* funcs) {
  if (!val.isObject()) {
    return NS_ERROR_INVALID_ARG;
  }

  // We might be adding functions to a Window.
  MC::Rooted<JSObject*> realIntlObj(
      cx, mc::CheckedUnwrapDynamic(&val.toObject(), cx));
  if (!realIntlObj) {
    return NS_ERROR_INVALID_ARG;
  }

  MC::SandboxStack<JSAutoRealm> ar(cx, realIntlObj);

  if (!JS_DefineFunctions(cx, realIntlObj, funcs)) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

NS_IMETHODIMP
MozIntlHelper::AddGetCalendarInfo(JS::Handle<JS::Value> val, MCContext* cx) {
  static const JSFunctionSpec funcs[] = {
      JS_SELF_HOSTED_FN("getCalendarInfo", "Intl_getCalendarInfo", 1, 0),
      JS_FS_END};

  return AddFunctions(cx, val, funcs);
}

NS_IMETHODIMP
MozIntlHelper::AddDateTimeFormatConstructor(JS::Handle<JS::Value> val,
                                            MCContext* cx) {
  if (!val.isObject()) {
    return NS_ERROR_INVALID_ARG;
  }

  // We might be adding this constructor to a Window
  MC::Rooted<JSObject*> realIntlObj(
      cx, mc::CheckedUnwrapDynamic(&val.toObject(), cx));
  if (!realIntlObj) {
    return NS_ERROR_INVALID_ARG;
  }

  MC::SandboxStack<JSAutoRealm> ar(cx, realIntlObj);

  if (!JS::AddMozDateTimeFormatConstructor(cx, realIntlObj)) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

NS_IMETHODIMP
MozIntlHelper::AddDisplayNamesConstructor(JS::Handle<JS::Value> val,
                                          MCContext* cx) {
  if (!val.isObject()) {
    return NS_ERROR_INVALID_ARG;
  }

  // We might be adding this constructor to a Window
  MC::Rooted<JSObject*> realIntlObj(
      cx, mc::CheckedUnwrapDynamic(&val.toObject(), cx));
  if (!realIntlObj) {
    return NS_ERROR_INVALID_ARG;
  }

  MC::SandboxStack<JSAutoRealm> ar(cx, realIntlObj);

  if (!JS::AddMozDisplayNamesConstructor(cx, realIntlObj)) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}
