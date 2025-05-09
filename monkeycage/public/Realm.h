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

class MOZ_RAII MCAutoRealm {
  JSAutoRealm inner_;

 public:
  MCAutoRealm(MCContext* cx, JSObject* target) : inner_(cx->cx_, target) {}
  MCAutoRealm(MCContext* cx, JSScript* target) : inner_(cx->cx_, target) {}
  ~MCAutoRealm() = default;
};
#else
using MCAutoRealm = JSAutoRealm;
#endif

#endif
