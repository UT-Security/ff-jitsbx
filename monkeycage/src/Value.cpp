/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "monkeycage/Value.h"

#ifdef JS_SANDBOX

namespace MC {

const JS::HandleValue& NullHandleValue() {
  static const JS::HandleValue inner_ =
    JS::HandleValue::fromMarkedLocation(JS::JSVAL_NULL_ADDRESS());
  return inner_;
}

const JS::HandleValue& UndefinedHandleValue() {
  static const JS::HandleValue inner_ =
    JS::HandleValue::fromMarkedLocation(JS::JSVAL_VOID_ADDRESS());
  return inner_;
}

const JS::HandleValue& TrueHandleValue() {
  static const JS::HandleValue inner_ =
    JS::HandleValue::fromMarkedLocation(JS::JSVAL_TRUE_ADDRESS());
  return inner_;  
}

const JS::HandleValue& FalseHandleValue() {
  static const JS::HandleValue inner_ =
    JS::HandleValue::fromMarkedLocation(JS::JSVAL_FALSE_ADDRESS());
  return inner_;  
}

const JS::Handle<mozilla::Maybe<JS::Value>>& NothingHandleValue() {
  static const JS::Handle<mozilla::Maybe<JS::Value>> inner_ =
    JS::Handle<mozilla::Maybe<JS::Value>>::fromMarkedLocation(JS::JSVAL_NOTHING_ADDRESS());
  return inner_;
}

}

#else

namespace MC {

const JS::HandleValue& NullHandleValue() {
  return JS::NullHandleValue; 
}

const JS::HandleValue& UndefinedHandleValue() {
  return JS::UndefinedHandleValue;  
}

const JS::HandleValue& TrueHandleValue() {
  return JS::TrueHandleValue;
}

const JS::HandleValue& FalseHandleValue() {
  return JS::FalseHandleValue;
}

const JS::Handle<mozilla::Maybe<JS::Value>>& NothingHandleValue() {
  return JS::NothingHandleValue;
}

}

#endif

