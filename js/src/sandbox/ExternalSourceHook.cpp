/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/sandbox/SourceHook.h"

using namespace js::sandbox;

SourceHook::SourceHook(const Ops* ops, void* sourceHook)
    : ops_(ops), sourceHook_(sourceHook) {}

SourceHook::~SourceHook() {
  ops_->destructor(sourceHook_);
}

bool SourceHook::load(JSContext* cx, const char* filename,
                      char16_t** twoByteSource, char** utf8Source,
                      size_t* length) {
  return ops_->load(sourceHook_, cx, filename, twoByteSource, utf8Source, length);
}
