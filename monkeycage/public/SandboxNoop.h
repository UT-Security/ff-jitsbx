/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_unsafe_SandboxNoop_h
#define mc_unsafe_SandboxNoop_h

#include <stdlib.h>

#include "monkeycage/unsafe/lib.h"
#include "monkeycage/SandboxCallback.h"

namespace MC {
namespace detail {
class SandboxNoop {
 public:
  static inline bool Initialize() {
    monkeycage_init();
    return true;
  }

  template <typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return external_addr;
  }

  template <typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template <typename T_Ret, typename... T_Args>
  static T_Cb<T_Ret, T_Args...> RegisterCallback(
      T_Cb<T_Ret, T_Args...> app_callback, size_t* index) {
    return reinterpret_cast<T_Cb<T_Ret, T_Args...>>(monkeycage_register_cb((void*)app_callback, index));
  }

  template <typename T_Ret, typename... T_Args>
  static T_Cb<T_Ret, T_Args...> RetrieveCallback(
      T_Cb<T_Ret, T_Args...> sbx_callback, size_t* index) {
    if (!sbx_callback) {
      *index = 40960;
      return nullptr;
    }
    return reinterpret_cast<T_Cb<T_Ret, T_Args...>>(
        monkeycage_retrieve_cb((void*)sbx_callback, index));
  }

  static size_t LastCallbackInvoked() {
    return monkeycage_last_callback_invoked;
  }
};

}  // namespace detail
}  // namespace MC
#endif
