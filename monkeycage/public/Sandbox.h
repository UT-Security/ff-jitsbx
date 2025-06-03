/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Sandbox_h
#define mc_Sandbox_h

#include "monkeycage/unsafe/SandboxImpl.h"
#include "monkeycage/SandboxCallback.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace MC {
namespace detail {

template<typename MC_Sbx>
class Sandbox {
private:
  static inline std::atomic_flag initialize_ = ATOMIC_FLAG_INIT;

  static inline std::shared_mutex callback_mutex;
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

  template<typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template<typename T>
  using Callback = MC::detail::SandboxCallback<T>;

  template<typename T_Ret, typename... T_Args>
  static Callback<T_Cb<T_Ret, T_Args...>> RegisterCallback(T_Cb<T_Ret, T_Args...> app_callback) {
    std::unique_lock<std::shared_mutex> guard(callback_mutex);
    return MC_Sbx::RegisterCallback(app_callback);
  }

  template<typename T_Ret, typename... T_Args>
  static Callback<T_Cb<T_Ret, T_Args...>> RetrieveCallback(T_Cb<T_Ret, T_Args...> sbx_callback) {    
    std::unique_lock<std::shared_mutex> guard(callback_mutex);
    return MC_Sbx::RetrieveCallback(sbx_callback);
  }
  
};
}

#if defined(JS_SANDBOX_NOOP)
using Sandbox = detail::Sandbox<detail::SandboxNoop>;
#elif defined(JS_SANDBOX_DYLIB)
using Sandbox = detail::Sandbox<detail::SandboxDylib>;
#elif defined(JS_SANDBOX_LFI)
using Sandbox = detail::Sandbox<detail::SandboxLFI>;
#else
using Sandbox = detail::Sandbox<detail::SandboxNone>;
#endif
}

#endif
