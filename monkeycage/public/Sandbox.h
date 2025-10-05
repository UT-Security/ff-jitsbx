/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Sandbox_h
#define mc_Sandbox_h

#include "SandboxTraits.h"
#include "monkeycage/SandboxCallback.h"
#include "monkeycage/SandboxHelpers.h"
#include "monkeycage/unsafe/SandboxImpl.h"
#include "monkeycage/SandboxTraits.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>

#include "mozilla/Assertions.h"

namespace MC {
namespace detail {

template<typename MC_Sbx>
class Sandbox {
private:
  static inline std::atomic_flag initialize_ = ATOMIC_FLAG_INIT;

  static inline std::shared_mutex callback_mutex;

  static inline void* callback_index_to_app_func[MC_Sbx::MAX_CALLBACKS] = {};
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

  template<typename T_Ret, typename... T_Args>
  using T_Cb_no_wrap = mc_remove_wrapper_t<T_Ret> (*)(mc_remove_wrapper_t<T_Args>...);

  template<typename T>
  using Callback = MC::detail::SandboxCallback<T>;

  template<typename T_Ret, typename... T_Args>
  static Callback<T_Cb<T_Ret, T_Args...>> RegisterCallback(T_Cb<T_Ret, T_Args...> app_callback) {
    std::unique_lock<std::shared_mutex> guard(callback_mutex);
    size_t index;
    T_Cb<T_Ret, T_Args...> sbx_callback = MC_Sbx::RegisterCallback(app_callback, (void*)app_callback, &index);
    callback_index_to_app_func[index] = (void*)app_callback;
    return Callback<T_Cb<T_Ret, T_Args...>>(nullptr, sbx_callback);
  }

  template <typename T_Ret, typename... T_Args>
  static Callback<T_Cb<T_Ret, T_Args...>> RetrieveCallback(
      T_Cb<T_Ret, T_Args...> sbx_callback) {
    using T_Func_Ret =
        std::conditional_t<std::is_void_v<T_Ret>, void, Tainted<T_Ret, MC_Sbx>>;
    using T_Func = T_Func_Ret (*)(mc_tainted_callback_arg_t<T_Args, MC_Sbx>...);
    std::unique_lock<std::shared_mutex> guard(callback_mutex);
    size_t index;

    MC_Sbx::RetrieveCallback(sbx_callback, &index);
    if (index == MC_Sbx::MAX_CALLBACKS) {
      return Callback<T_Cb<T_Ret, T_Args...>>(nullptr);
    }

    auto app_callback =
        reinterpret_cast<T_Func>(callback_index_to_app_func[index]);
    return Callback<T_Cb<T_Ret, T_Args...>>(app_callback, sbx_callback);
  }

  template <typename T_Arg>
  static inline mc_tainted_callback_arg_t<T_Arg, MC_Sbx> CallbackInterceptorConvertParam(T_Arg&& arg) {
    if_constexpr_named (cond1, is_fundamental_or_enum_v<T_Arg>) {
      return arg;
    } else if_constexpr_named(cond2, std::is_class_v<T_Arg>) {
      return arg;
    } else if_constexpr_named(cond3, std::is_pointer_v<T_Arg> && std::is_void_v<std::remove_pointer_t<T_Arg>>) {
      return AppPointer<void*, MC_Sbx>(arg);
    } else if_constexpr_named(cond4, std::is_pointer_v<T_Arg>) {
      Tainted<T_Arg, MC_Sbx> ret(nullptr);
      ret.assign_raw_pointer(arg);
      return ret;
    } else if_constexpr_named(cond5, std::is_lvalue_reference_v<T_Arg>) {
      return arg;
    } else {
      constexpr auto unknownCase = !(cond1 || cond2 || cond3 || cond4 || cond5);
      mc_detail_static_fail_because(unknownCase, "Unknown case for callback interceptor parameter");
    }
  }

  template <typename T_Ret, typename... T_Args>
  static T_Ret CallbackInterceptor(T_Args... params) {
    using T_Func_Ret =
        std::conditional_t<std::is_void_v<T_Ret>, void, Tainted<T_Ret, MC_Sbx>>;
    using T_Func = T_Func_Ret (*)(mc_tainted_callback_arg_t<T_Args, MC_Sbx>...);

    auto app_callback = reinterpret_cast<T_Func>(callback_index_to_app_func[MC_Sbx::InvokedCallback()]);

    if constexpr (std::is_void_v<T_Ret>) {
      app_callback(CallbackInterceptorConvertParam<T_Args>(std::forward<T_Args>(params))...);
    } else {
      auto tainted_ret = app_callback(CallbackInterceptorConvertParam<T_Args>(std::forward<T_Args>(params))...);
      return tainted_ret.UNSAFE_unverified();
    }
  }

  template <typename T_Ret, typename... T_Args>
  static Callback<T_Cb_no_wrap<T_Ret, T_Args...>> RegisterTaintedCallback(
      T_Ret (*app_callback)(T_Args...)) {

    // Some branches don't use this param.
    MC_UNUSED(app_callback);

    if_constexpr_named(cond1, !(mc_is_tainted_callback_arg_v<T_Args> && ...))
    {
      mc_detail_static_fail_because(
        cond1,
        "Change all pointer arguments to be Tainted."
      );
    } else if_constexpr_named(cond2, !(std::is_void_v<T_Ret> || mc_is_Tainted_v<T_Ret>))
    {
      mc_detail_static_fail_because(
        cond2,
        "Change callback return type to be Tainted if it is not void."
      );
    }
    else {
      std::unique_lock<std::shared_mutex> guard(callback_mutex);
      size_t index;

      auto callback_interceptor =
          CallbackInterceptor<mc_remove_wrapper_t<T_Ret>,
                              mc_remove_wrapper_t<T_Args>...>;

      T_Cb_no_wrap<T_Ret, T_Args...> sbx_callback =
          MC_Sbx::RegisterCallback(callback_interceptor, (void*)app_callback, &index);
      callback_index_to_app_func[index] = (void*)app_callback;
      return Callback<T_Cb_no_wrap<T_Ret, T_Args...>>(app_callback, sbx_callback);
    }
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
