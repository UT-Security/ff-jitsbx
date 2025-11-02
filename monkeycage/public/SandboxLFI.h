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

#include "lfi.h"
#include "lfi_tux.h"
#include "monkeycage/unsafe/lib.h"

#include "mozilla/Assertions.h"

namespace MC {
namespace detail {

extern "C" void* lfi_libcalls(void);

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

  static inline std::atomic_flag hasSIGSEGVHandler = ATOMIC_FLAG_INIT;
  static inline struct sigaction sPrevSIGSEGVHandler;

  static inline std::atomic_flag hasSIGILLHandler = ATOMIC_FLAG_INIT;
  static inline struct sigaction sPrevSIGILLHandler;

  static void OnSignal(int signum, siginfo_t* info, void* context) {
    struct TuxThread* p = lfi_tux_get_thread();
    if(lfi_tux_on_signal(p, signum, info, context)) {
      return;
    }

    struct sigaction* previousSignal = nullptr;
    switch (signum) {
      case SIGSEGV:
        previousSignal = &sPrevSIGSEGVHandler;
        break;
      case SIGILL:
        previousSignal = &sPrevSIGILLHandler;
        break;
    }
    MOZ_ASSERT(previousSignal);

    if (previousSignal->sa_flags & SA_SIGINFO) {
      previousSignal->sa_sigaction(signum, info, context);
    } else if (previousSignal->sa_handler == SIG_DFL ||
               previousSignal->sa_handler == SIG_IGN) {
      sigaction(signum, previousSignal, nullptr);
    } else {
      previousSignal->sa_handler(signum);
    }
  }

  static bool OnSigaction(int signum) {
    if (signum == SIGSEGV) {
      if(hasSIGSEGVHandler.test_and_set()) return true;
      struct sigaction faultHandler;
      faultHandler.sa_flags = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
      faultHandler.sa_sigaction = OnSignal;
      sigemptyset(&faultHandler.sa_mask);
      if (sigaction(SIGSEGV, &faultHandler, &sPrevSIGSEGVHandler)) {
        MOZ_CRASH("unable to install monkeycage sigsegv handler");
      }

      return true;
    } else if (signum == SIGILL) {
      if(hasSIGILLHandler.test_and_set()) return true;
      struct sigaction wasmTrapHandler;
      wasmTrapHandler.sa_flags = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
      wasmTrapHandler.sa_sigaction = OnSignal;
      sigemptyset(&wasmTrapHandler.sa_mask);
      if (sigaction(SIGILL, &wasmTrapHandler, &sPrevSIGILLHandler)) {
        MOZ_CRASH("unable to install monkeycage sigill handler");
      }

      return true;
    }

    return false;
  }

 public:
  static bool Initialize() {
    monkeycage_init(lfi_libcalls(), (void*)OnSigaction);
    return true;
  }

  template <typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return (T_Fn)(monkeycage_addr((void*)external_addr));
  }

  template <typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  //(NOTE): Should be kept in sync with lfi-bind
  static constexpr size_t MAX_CALLBACKS = 40960;

  template <typename T_Ret, typename... T_Args>
  static T_Cb<T_Ret, T_Args...> RegisterCallback(
      T_Cb<T_Ret, T_Args...> app_callback, void* key, size_t* index) {
    size_t stack_args_size = CallbackStackArgs<0, 0, T_Ret, T_Args...>(
        0, reinterpret_cast<T_Ret (*)(T_Args...)>(0));
    return reinterpret_cast<T_Cb<T_Ret, T_Args...>>(monkeycage_register_cb(
        (void*)app_callback, stack_args_size, key, index));
  }

  template <typename T_Ret, typename... T_Args>
  static T_Cb<T_Ret, T_Args...> RetrieveCallback(
      T_Cb<T_Ret, T_Args...> sbx_callback, size_t* index) {
    if (!sbx_callback) {
      *index = MAX_CALLBACKS;
      return nullptr;
    }
    return reinterpret_cast<T_Cb<T_Ret, T_Args...>>(
        monkeycage_retrieve_cb((void*)sbx_callback, index));
  }

  static size_t InvokedCallback() { return monkeycage_invoked_cb(); }

};

using SandboxImpl = SandboxLFI;

}  // namespace detail
}  // namespace MC


#endif
