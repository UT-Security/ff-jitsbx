/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jitsbx/JitSandboxActivation.h"
#include "jitsbx/JitSandbox.h"
#include "vm/JitActivation.h"
#include "vm/JSContext.h"

using namespace js;
using namespace js::jitsbx;

JitSandboxActivation::JitSandboxActivation(JSContext* cx,
                                           jit::JitActivation* prev)
    : cx_(cx), prev_(prev) {
#ifdef JITSBX_CFI_STACK
  if (prev_ != nullptr) {
    prev_->jitSandboxActivation()->setSavedNativeStackPtr(
        cx_->runtime()->jitSandbox()->savedNativeStackPtr());
    prev_->jitSandboxActivation()->setSavedSandboxStackPtr(
        cx_->runtime()->jitSandbox()->savedSandboxStackPtr());
  }
#endif
}

JitSandboxActivation::~JitSandboxActivation() {
#ifdef JITSBX_CFI_STACK
  if (prev_ != nullptr) {
    cx_->runtime()->jitSandbox()->setSavedNativeStackPtr(
        prev_->jitSandboxActivation()->savedNativeStackPtr());
    cx_->runtime()->jitSandbox()->setSavedSandboxStackPtr(
        prev_->jitSandboxActivation()->savedSandboxStackPtr());
    prev_->jitSandboxActivation()->setSavedNativeStackPtr(nullptr);
    prev_->jitSandboxActivation()->setSavedSandboxStackPtr(nullptr);
  } else {
    cx_->runtime()->jitSandbox()->setSavedNativeStackPtr(nullptr);
    cx_->runtime()->jitSandbox()->setSavedSandboxStackPtr(
        (uint8_t*)cx_->runtime()->jitSandbox()->sandboxStackBase());
  }
#endif
}
