/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxCallback_h
#define mc_SandboxCallback_h

#if defined(JS_SANDBOX)

#include "monkeycage/unsafe/SandboxImpl.h"
#include "monkeycage/SandboxTraits.h"

namespace MC {
namespace detail {

template<typename MC_Sbx>
class Sandbox;

template <typename T>
class SandboxCallback;

template <typename T_Ret, typename... T_Args>
class SandboxCallback<T_Ret(*)(T_Args...)> {
  using T_Sbx_Cb = T_Ret (*)(T_Args...);
  using T_App_Cb_Ret = std::conditional_t<std::is_void_v<T_Ret>, void, Tainted<T_Ret, SandboxImpl>>;
  using T_App_Cb = T_App_Cb_Ret (*)(mc_tainted_callback_arg_t<T_Args, SandboxImpl>...);
  
  T_App_Cb app_fn_;
  T_Sbx_Cb sbx_fn_;

  template<typename T_Sbx>
  friend class Sandbox;

 public:
  explicit SandboxCallback(T_App_Cb app_fn, T_Sbx_Cb sbx_fn) : app_fn_(app_fn), sbx_fn_(sbx_fn) {}

  SandboxCallback(const std::nullptr_t& arg) : app_fn_(arg), sbx_fn_(arg) {}

  T_Sbx_Cb UNSAFE_get() const { return sbx_fn_; }

  operator bool() const { return app_fn_ == nullptr ? false : true; }

  template<typename... Args>
  T_App_Cb_Ret operator()(Args&&... args) {
    return app_fn_(std::forward<Args>(args)...);
  }
};
}

template <typename T>
using SandboxCallback = detail::SandboxCallback<T>;
}
#endif
#endif
