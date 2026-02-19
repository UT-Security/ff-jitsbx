/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_unsafe_SandboxLFI_h
#define mc_unsafe_SandboxLFI_h

#include <atomic>
#include <signal.h>
#include <stdlib.h>
#include <type_traits>

#include "monkeycage/unsafe/monkeycage.h"

#include "mozilla/Assertions.h"

namespace MC {
namespace detail {

class SandboxLFI {
  template <size_t T_NumIntegerArgs, size_t T_NumFloatArgs, typename T_Ret>
  static inline size_t CallbackStackArgs(size_t stack_args_size, T_Ret (*)()) {
    return stack_args_size;
  }

  template <size_t T_NumIntegerArgs, size_t T_NumFloatArgs, typename T_Ret,
            typename T_Arg, typename... T_Args>
  static inline size_t CallbackStackArgs(size_t stack_args_size,
                                         T_Ret (*)(T_Arg, T_Args...)) {
    if constexpr (std::is_same_v<T_Arg, float> ||
                  std::is_same_v<T_Arg, double>) {
      if constexpr (T_NumFloatArgs < 8) {
        return CallbackStackArgs<T_NumIntegerArgs, T_NumFloatArgs + 1>(
            stack_args_size, reinterpret_cast<T_Ret (*)(T_Args...)>(0));
      } else {
        return CallbackStackArgs<T_NumIntegerArgs, T_NumFloatArgs + 1>(
            stack_args_size + sizeof(T_Arg),
            reinterpret_cast<T_Ret (*)(T_Args...)>(0));
      }
    } else {
      if constexpr (T_NumIntegerArgs < 6) {
        return CallbackStackArgs<T_NumIntegerArgs + 1, T_NumFloatArgs>(
            stack_args_size, reinterpret_cast<T_Ret (*)(T_Args...)>(0));
      } else {
        return CallbackStackArgs<T_NumIntegerArgs + 1, T_NumFloatArgs>(
            stack_args_size + sizeof(T_Arg),
            reinterpret_cast<T_Ret (*)(T_Args...)>(0));
      }
    }
  }

 public:
  static bool Initialize() {
    monkeycage_init();
    return true;
  }

  template <typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return (T_Fn)(monkeycage_addr((void*)external_addr));
  }

  template <typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template <typename T_Ret, typename... T_Args>
  static T_Cb<T_Ret, T_Args...> RegisterCallback(
      void* key, T_Cb<T_Ret, T_Args...> app_callback) {
    size_t stack_args_size = CallbackStackArgs<0, 0, T_Ret, T_Args...>(
        0, reinterpret_cast<T_Ret (*)(T_Args...)>(0));
    return reinterpret_cast<T_Cb<T_Ret, T_Args...>>(
        lfi_box_register_cb_key(monkeycage_box, key, (void*)app_callback,
                                     stack_args_size));
  }

  template <typename T_Ret, typename... T_Args>
  static void* RetrieveCallback(T_Cb<T_Ret, T_Args...> sbx_callback) {
    if (!sbx_callback) {
      return nullptr;
    }
    return lfi_box_lookup_cb(monkeycage_box, (void*)sbx_callback);
  }

  static void* InvokedCallbackData() { return lfi_ctx_last_cb_key(monkeycage_ctx); }

};

using SandboxImpl = SandboxLFI;

}  // namespace detail
}  // namespace MC


#endif
