/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_unsafe_SandboxDylib_h
#define mc_unsafe_SandboxDylib_h

#include "monkeycage/unsafe/lib.h"
#include "monkeycage/SandboxCallback.h"

namespace MC {
namespace detail {
class SandboxDylib {
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
  static MC::SandboxCallback<T_Cb<T_Ret, T_Args...>> RegisterCallback(
      T_Cb<T_Ret, T_Args...> app_callback) {
    return MC::SandboxCallback<T_Cb<T_Ret, T_Args...>>(
        app_callback, reinterpret_cast<T_Cb<T_Ret, T_Args...>>(
                      monkeycage_register_cb((void*)app_callback, 0)));
  }

  template <typename T_Ret, typename... T_Args>
  static MC::SandboxCallback<T_Cb<T_Ret, T_Args...>> RetrieveCallback(
      T_Cb<T_Ret, T_Args...> sbx_callback) {
    return sbx_callback ? MC::SandboxCallback<T_Cb<T_Ret, T_Args...>>(
        reinterpret_cast<T_Cb<T_Ret, T_Args...>>(
            monkeycage_retrieve_cb((void*)sbx_callback)),
        sbx_callback) :  MC::SandboxCallback<T_Cb<T_Ret, T_Args...>>{nullptr};
  }
};
}  // namespace detail
}  // namespace MC
#endif
