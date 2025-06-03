/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "js/Principals.h"

sandbox::JSPrincipals::JSPrincipals(const Ops* ops, void* principals)
    : ops_(ops), principals_(principals) {}

bool sandbox::JSPrincipals::write(JSContext* cx,
                                  JSStructuredCloneWriter* writer) {
  return ops_->write(principals_, cx, writer);
}

bool sandbox::JSPrincipals::isSystemOrAddonPrincipal() {
  return ops_->isSystemOrAddonPrincipal(principals_);
}
