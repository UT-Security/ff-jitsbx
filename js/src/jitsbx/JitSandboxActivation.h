/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jitsbx_Activation_h
#define jitsbx_Activation_h

#include "jstypes.h"

#include "vm/JSContext.h"

namespace js {

namespace jit {
class JitActivation;
}  // namespace jit

namespace jitsbx {

class JitSandboxActivation {
 private:
  JSContext* cx_;
  jit::JitActivation* prev_;
#ifdef JITSBX_CFI_STACK
 private:
  const uint8_t* savedNativeStackPtr_{nullptr};
  const uint8_t* savedSandboxStackPtr_{nullptr};

 public:
  const uint8_t* savedNativeStackPtr() const { return savedNativeStackPtr_; }
  void setSavedNativeStackPtr(const uint8_t* ptr) {
    savedNativeStackPtr_ = ptr;
  }

  const uint8_t* savedSandboxStackPtr() const { return savedSandboxStackPtr_; }
  void setSavedSandboxStackPtr(const uint8_t* ptr) {
    savedSandboxStackPtr_ = ptr;
  }
#endif

 public:
  JitSandboxActivation(JSContext* cx, jit::JitActivation* prev);
  ~JitSandboxActivation();
};

}  // namespace jitsbx

}  // namespace js
#endif
