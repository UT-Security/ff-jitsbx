/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Sandbox_h
#define monkeycage_Sandbox_h

#include <atomic>
#include <mutex>
#include "js/sandbox/lib.h"
#include "mozilla/Assertions.h"

namespace monkeycage {

template <typename T>
class SandboxCallback;

class Sandbox {
private:
  static inline std::atomic_flag initialized_ = ATOMIC_FLAG_INIT;
public:
  static bool Initialize() {
    if (initialized_.test_and_set()) {
      // sandbox is already initialized / in the process of being initialized.
      return false;
    }
    sbx_init();
    return true;
  }

  template <typename T>
  static SandboxCallback<T> RegisterCallback(T callback) {
    MOZ_ASSERT(initialized_.test_and_set(),
               "MONKEYCAGE: Attempt to register callback before sandbox "
               "initialization");

    return SandboxCallback<T>((T)sbx_register_cb((void*)callback, 0));
  }
};

template <typename T>
class SandboxCallback {
private:
  T callback_trampoline_;

public:
  SandboxCallback(T callback_trampoline): callback_trampoline_(callback_trampoline) {}
  
  void set(T callback_trampoline) { callback_trampoline_ = callback_trampoline; }
  T get() { return callback_trampoline_; }
};

}

#endif
