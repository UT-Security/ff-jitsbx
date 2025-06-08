/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxCallback_h
#define mc_SandboxCallback_h

namespace MC {
namespace detail {
#if defined(JS_SANDBOX_NOOP)

class SandboxNoop;

template <typename T>
class SandboxCallback;

template <typename T_Ret, typename... T_Args>
class SandboxCallback<T_Ret(*)(T_Args...)> {
  using T_Cb = T_Ret (*)(T_Args...);
  T_Cb fn_;

  friend class SandboxNoop;
  explicit SandboxCallback(T_Cb fn) : fn_(fn) {}

 public:
  SandboxCallback(const std::nullptr_t& arg) : fn_(arg) {}

  T_Cb UNSAFE_get() const { return fn_; }

  operator bool() const { return fn_ == nullptr ? false : true; }

  template <typename... Args>
  T_Ret operator()(Args&&... args) {
    return fn_(std::forward<Args>(args)...);
  }
};
#elif defined(JS_SANDBOX_DYLIB)

class SandboxDylib;

template <typename T>
class SandboxCallback;

template <typename T_Ret, typename... T_Args>
class SandboxCallback<T_Ret(*)(T_Args...)> {
  using T_Cb = T_Ret (*)(T_Args...);
  
  T_Cb fn_;

  friend class SandboxDylib;
  explicit SandboxCallback(T_Cb fn) : fn_(fn) {}

 public:
  SandboxCallback(const std::nullptr_t& arg) : fn_(arg) {}

  T_Cb UNSAFE_get() const { return fn_; }

  operator bool() const { return fn_ == nullptr ? false : true; }

  template <typename... Args>
  T_Ret operator()(Args&&... args) {
    return fn_(std::forward<Args>(args)...);
  }
};
#elif defined(JS_SANDBOX_LFI)

class SandboxLFI;

template <typename T>
class SandboxCallback;

template <typename T_Ret, typename... T_Args>
class SandboxCallback<T_Ret(*)(T_Args...)> {
  using T_Cb = T_Ret (*)(T_Args...);
  
  T_Cb app_fn_;
  T_Cb sbx_fn_;

  friend class SandboxLFI;
  explicit SandboxCallback(T_Cb app_fn, T_Cb sbx_fn) : app_fn_(app_fn), sbx_fn_(sbx_fn) {}

 public:
  SandboxCallback(const std::nullptr_t& arg) : app_fn_(arg), sbx_fn_(arg) {}

  T_Cb UNSAFE_get() const { return sbx_fn_; }

  operator bool() const { return app_fn_ == nullptr ? false : true; }

  template <typename... Args>
  T_Ret operator()(Args&&... args) {
    return app_fn_(std::forward<Args>(args)...);
  }
};

#else
template <typename T>
using SandboxCallback = T;
#endif
}
template <typename T>
using SandboxCallback = MC::detail::SandboxCallback<T>;
}
#endif
