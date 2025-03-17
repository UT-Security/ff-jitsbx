/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Sandbox_h
#define monkeycage_Sandbox_h

#include <atomic>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>

#include "js/sandbox/lib.h"
#include "mozilla/Assertions.h"

namespace monkeycage {

namespace detail {

// https://stackoverflow.com/questions/37602057/why-isnt-a-for-loop-a-compile-time-expression
namespace compile_time_for_detail {
template <std::size_t N>
struct num {
  static const constexpr auto value = N;
};

template <class F, std::size_t... Is>
inline void compile_time_for_helper(F func, std::index_sequence<Is...>) {
  (func(num<Is>{}), ...);
}
}  // namespace compile_time_for_detail

template <std::size_t N, typename F>
inline void compile_time_for(F func) {
  compile_time_for_detail::compile_time_for_helper(
      func, std::make_index_sequence<N>());
}

}  // namespace detail

template <typename T>
class SandboxCallback;

class Sandbox {
private:
  static inline std::atomic_flag initialized = ATOMIC_FLAG_INIT;
  
  static inline std::shared_mutex callback_mutex;
  static inline const size_t MAX_CALLBACKS = 40960;
  static inline thread_local size_t last_callback = 0; 
  static inline void* callback_targets[MAX_CALLBACKS]{ nullptr };
  static inline void* callback_interceptors[MAX_CALLBACKS]{ nullptr };
  
public:
  static bool Initialize() {
    if (initialized.test_and_set()) {
      // sandbox is already initialized / in the process of being initialized.
      return false;
    }
    sbx_init();
    return true;
  }

  template<typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template<typename T_Ret, typename... T_Args>
  using T_Cb_no_wrap = T_Ret (*)(T_Args...);

  template<size_t N, typename T_Ret, typename... T_Args>
  static T_Ret CallbackTrampoline(T_Args... params) {
    last_callback = N;
    T_Cb<T_Ret, T_Args...> interceptor;
    {
      //std::shared_lock lock(callback_mutex);
      interceptor = reinterpret_cast<T_Cb<T_Ret, T_Args...>>(callback_interceptors[N]);
    }

    return interceptor(params...);
  }

  template<typename T_Ret, typename... T_Args>
  static T_Ret CallbackInterceptor(T_Args... params) {
    size_t index = last_callback;
    auto target = reinterpret_cast<T_Cb<T_Ret, T_Args...>>(callback_targets[index]);

    return target(params...);
  }
  
  template <typename T_Ret, typename... T_Args>
  static SandboxCallback<T_Cb<T_Ret, T_Args...>> RegisterCallback(
      T_Cb<T_Ret, T_Args...> callback) {
    MOZ_ASSERT(initialized.test_and_set(),
               "MONKEYCAGE: Attempt to register callback before sandbox "
               "initialization");

    std::unique_lock<std::shared_mutex> guard(callback_mutex);

    return SandboxCallback<T_Cb<T_Ret, T_Args...>>(
        reinterpret_cast<T_Cb<T_Ret, T_Args...>>(
            sbx_register_cb(reinterpret_cast<void*>(callback), 0)));
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
