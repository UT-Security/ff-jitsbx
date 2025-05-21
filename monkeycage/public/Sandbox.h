/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Sandbox_h
#define mc_Sandbox_h

#include <atomic>
#include <mutex>
#include <shared_mutex>

#if defined(JS_SANDBOX_NOOP)
namespace MC {
namespace detail {

template <typename T>
class SandboxCallback {
  T fn_;

  friend class SandboxNoop;
  explicit SandboxCallback(T fn): fn_(fn) {}
public:
  explicit SandboxCallback(const std::nullptr_t& arg): fn_(arg) {}

  T UNSAFE_get() const { return fn_; }
};

class SandboxNoop {
public:
  static inline bool Initialize() {
    return true;
  }

  template<typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return external_addr;
  }

  template<typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template<typename T_Ret, typename... T_Args>
  static SandboxCallback<T_Cb<T_Ret, T_Args...>> RegisterCallback(T_Cb<T_Ret, T_Args...> callback) {
    return SandboxCallback<T_Cb<T_Ret, T_Args...>>(callback);
  }
};

}
}
#elif defined(JS_SANDBOX_DYLIB)
#include "monkeycage/unsafe/lib.h"

namespace MC {
namespace detail {

template <typename T>
class SandboxCallback {
  T fn_;

  friend class SandboxDylib;
  explicit SandboxCallback(T fn): fn_(fn) {}
public:
  explicit SandboxCallback(const std::nullptr_t& arg): fn_(arg) {}

  T UNSAFE_get() const { return fn_; }
};

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

  template<typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template<typename T_Ret, typename... T_Args>
  static SandboxCallback<T_Cb<T_Ret, T_Args...>> RegisterCallback(T_Cb<T_Ret, T_Args...> callback) {
    return SandboxCallback<T_Cb<T_Ret, T_Args...>>(callback);
  }
  
};

}
}
#else

namespace MC {
namespace detail {

template <typename T>
using SandboxCallback = T;

class SandboxNone {
public:
  static inline bool Initialize() {
    return true;
  }

  template<typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return external_addr;
  }

  template<typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template<typename T_Ret, typename... T_Args>
  static SandboxCallback<T_Cb<T_Ret, T_Args...>> RegisterCallback(T_Cb<T_Ret, T_Args...> callback) {
    return callback;
  }
};

}
}
#endif

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
  static Callback<T_Cb<T_Ret, T_Args...>> RegisterCallback(T_Cb<T_Ret, T_Args...> callback) {
    std::unique_lock<std::shared_mutex> guard(callback_mutex);
    return MC_Sbx::RegisterCallback(callback);
  }
  
};

}

#if defined(JS_SANDBOX_NOOP)
using Sandbox = detail::Sandbox<detail::SandboxNoop>;
#elif defined(JS_SANDBOX_DYLIB)
using Sandbox = detail::Sandbox<detail::SandboxDylib>;
#else
using Sandbox = detail::Sandbox<detail::SandboxNone>;
#endif

}

#endif
