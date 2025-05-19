/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Sandbox_h
#define mc_Sandbox_h

#include <atomic>

namespace MC {
namespace detail {

template<typename MC_Sbx>
class Sandbox {
private:
  static inline std::atomic_flag initialize_ = ATOMIC_FLAG_INIT;
public:
  static bool Initialize() {
    if (initialize_.test_and_set()) {
      return false;
    }
    MC_Sbx::Initialize();
    return true;
  }

  template<typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return MC_Sbx::Address(external_addr);
  }
};

}
}

#ifdef JS_SANDBOX_DYLIB
#include "monkeycage/unsafe/lib.h"

namespace MC {
namespace detail {

class SandboxDylib {
public:
  static bool Initialize() {
    monkeycage_init();
    return true;
  }

  template<typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return (T_Fn)(monkeycage_addr((void*)external_addr));
  }
};
}

using Sandbox = detail::Sandbox<detail::SandboxDylib>;

}
#else

namespace MC {
namespace detail {

class SandboxNoop {
public:
  static inline bool Initialize() {
    return true;
  }

  template<typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return external_addr;
  }
};

}

using Sandbox = detail::Sandbox<detail::SandboxNoop>;

}

#endif

#endif
