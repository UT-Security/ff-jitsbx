/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/GCAPI.h"

sandbox::JSExternalStringCallbacks::JSExternalStringCallbacks(
    const Ops* ops, const void* callbacks)
    : ops_(ops), callbacks_(callbacks) {}

void sandbox::JSExternalStringCallbacks::finalize(char16_t* chars) const {
  return ops_->finalize(callbacks_, chars);
}

size_t sandbox::JSExternalStringCallbacks::sizeOfBuffer(
    const char16_t* chars, mozilla::MallocSizeOf mallocSizeOf) const {
  return ops_->sizeOfBuffer(callbacks_, chars, mallocSizeOf);
}
