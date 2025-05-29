/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Tainted_h
#define mc_Tainted_h

#include "monkeycage/unsafe/SandboxImpl.h"

#include <type_traits>
#include <utility>

namespace MC {

namespace detail {

template<typename T, typename MC_Sbx>
class Tainted;

template<typename T, typename MC_Sbx>
class TaintedVolatile;

template<template<typename, typename> typename T_Wrap, typename T, typename MC_Sbx>
class TaintedBase {
public:
  inline auto& impl() { return *static_cast<T_Wrap<T, MC_Sbx>*>(this); }
  inline auto& impl() const { return *static_cast<const T_Wrap<T, MC_Sbx>*>(this); }

  inline auto UNSAFE_unverified() const { return impl().get_raw_value(); }

private:
  using T_OpDerefRet = TaintedVolatile<std::remove_pointer_t<T>, MC_Sbx>;

public:
  inline T_OpDerefRet& operator*() {
    static_assert(std::is_pointer_v<T>, "Operator * only allowed on pointers");
    auto ret_ptr_const =
      reinterpret_cast<const T_OpDerefRet*>(impl().get_raw_value());
    // Safe - If T_OpDerefRet is not a const ptr, this is trivially safe
    //        If T_OpDerefRet is a const ptr, then the const is captured
    //        inside the wrapper
    auto ret_ptr = const_cast<T_OpDerefRet*>(ret_ptr_const);
    return *ret_ptr;
  }

  inline const T_OpDerefRet* operator->() const {
    static_assert(std::is_pointer_v<T>, "Operator -> only allowed on pointers");
    return reinterpret_cast<const T_OpDerefRet*>(impl().get_raw_value());
  }

  inline T_OpDerefRet* operator->() {
    return const_cast<T_OpDerefRet*>(std::as_const(*this).operator->());
  }
};

template<typename T, typename MC_Sbx>
class Tainted : public TaintedBase<Tainted, T, MC_Sbx> {
  // Classes receive their own specialization
  static_assert(
    !std::is_class_v<T>,
    "Missing definition for class T."
  );

  static_assert(
    std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>,
    "Tainted types support only fundamental, enum and pointer types."
  );

private:
  friend class TaintedBase<Tainted, T, MC_Sbx>;
  
  using T_ClassBase = TaintedBase<Tainted, T, MC_Sbx>;

  //TODO(abhishek): not picking representation based on MC_Sbx type.
  T data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

  inline std::remove_cv_t<T> get_raw_value() const noexcept {
    return data;
  }

  template<typename T2 = T, typename = std::enable_if_t<std::is_pointer_v<T>, T>>
  Tainted(T2 val, const void* /* internal tag */) : data(val) {
    // Sanity check
    static_assert(std::is_pointer_v<T>);
  }
public:
  Tainted() = default;
  Tainted(const Tainted<T, MC_Sbx>& p) = default;

  Tainted(const std::nullptr_t& arg) : data(arg) {
    static_assert(std::is_pointer_v<T>);
  }
  
};

template<typename T, typename MC_Sbx>
class TaintedVolatile : public TaintedBase<TaintedVolatile, T, MC_Sbx> {
  // Classes recieve their own specialization.
  static_assert(
    !std::is_class_v<T>,
    "Missing definition for class T."
  );

  static_assert(
    std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>,
    "Tainted types support only fundamental, enum and pointer types."
  );

private:
  friend class TaintedBase<TaintedVolatile, T, MC_Sbx>;
  
  using T_ClassBase = TaintedBase<TaintedVolatile, T, MC_Sbx>;

  //TODO(abhishek): not picking representation based on MC_Sbx type.
  T data;

  inline auto& get_sandbox_value_ref() noexcept { return data; }
  inline auto& get_sandbox_value_ref() const noexcept { return data; }

  TaintedVolatile() = default;
  TaintedVolatile(const TaintedVolatile<T, MC_Sbx>& p) = default;

public:
  
};

}
#if defined(JS_SANDBOX_NOOP)
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxNoop>;
#elif defined(JS_SANDBOX_DYLIB)
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxDylib>;
#elif defined(JS_SANDBOX_LFI)
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxLFI>;
#else
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxNone>;
#endif
}

#endif
