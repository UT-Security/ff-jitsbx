/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_unsafe_SandboxNoop_h
#define mc_unsafe_SandboxNoop_h

#include "monkeycage/SandboxCallback.h"

namespace MC {
namespace detail {
class SandboxNoop {
 public:
  static inline bool Initialize() { return true; }

  template <typename T_Fn>
  static inline T_Fn Address(T_Fn external_addr) {
    return external_addr;
  }

  template <typename T_Ret, typename... T_Args>
  using T_Cb = T_Ret (*)(T_Args...);

  template <typename T_Ret, typename... T_Args>
  static SandboxCallback<T_Cb<T_Ret, T_Args...>> RegisterCallback(
      T_Cb<T_Ret, T_Args...> callback) {
    return SandboxCallback<T_Cb<T_Ret, T_Args...>>(callback);
  }
};

}  // namespace detail
}  // namespace MC
#endif
