/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Tainted_h
#define mc_Tainted_h

#include "monkeycage/SandboxCallback.h"
#include "monkeycage/SandboxHelpers.h"
#include "monkeycage/unsafe/SandboxImpl.h"
#include "monkeycage/SandboxTraits.h"

#include <cstdint>
#include <type_traits>
#include <utility>

namespace MC {

namespace detail {

template<typename T, typename MC_Sbx>
class Tainted;

template<typename T, typename MC_Sbx>
class TaintedVolatile;

template<typename T, typename MC_Sbx>
class TaintedUnchecked;

template<template<typename, typename> typename T_Wrap, typename T, typename MC_Sbx>
class TaintedBase {
public:
  inline auto& impl() { return *static_cast<T_Wrap<T, MC_Sbx>*>(this); }
  inline auto& impl() const { return *static_cast<const T_Wrap<T, MC_Sbx>*>(this); }

  inline auto UNSAFE_unverified() const { return impl().get_raw_value(); }
  inline auto INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

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

  template<typename T_Func>
  inline auto copy_and_verify_address(T_Func verifier) const {
    static_assert(std::is_pointer_v<T>,
                  "copy_and_verify_address must be used on pointers.");

    auto val = reinterpret_cast<uintptr_t>(impl().get_raw_value());
    return verifier(val);
  }

  template<typename T_Func>
  inline auto copy_and_verify(T_Func verifier) const {
    //using T_Deref = std::remove_cv_t<std::remove_pointer_t<T>>;

    if_constexpr_named(cond1, is_fundamental_or_enum_v<T>) {
      auto val = impl().get_raw_value();
      return verifier(val);
    } else
    {
      constexpr bool unknownCase = !(cond1);
      mc_detail_static_fail_because(
          unknownCase, "copy_and_verify not supported for this type");
    }
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
  template<template<typename, typename> typename U1, typename U2, typename U3>
  friend class TaintedBase;
  
  template<typename U1, typename U2>
  friend class TaintedVolatile;

  template<typename U1, typename U2, typename U_Sbx>
  friend inline Tainted<U1, U_Sbx> tainted_static_cast(const Tainted<U2, U_Sbx>& rhs) noexcept;
  
  template<typename U1, typename U2, typename U_Sbx>
  friend inline Tainted<U1, U_Sbx> tainted_reinterpret_cast(const Tainted<U2, U_Sbx>& rhs) noexcept;
  
  using T_ClassBase = TaintedBase<Tainted, T, MC_Sbx>;

  T data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

  inline std::remove_cv_t<T> get_raw_value() const noexcept {
    return data;
  }

  template<typename T2 = T, typename = std::enable_if_t<std::is_pointer_v<T2>, T>>
  Tainted(T2 val, const void* /* internal tag */) : data(val) {
    // Sanity check
    static_assert(std::is_pointer_v<T>);
  }

  template <typename T_Rhs>
  static inline Tainted<T, MC_Sbx> internal_factory(T_Rhs&& rhs) {
    if constexpr (std::is_pointer_v<std::remove_reference_t<T_Rhs>>) {
      const void* internal_tag = nullptr;
      return Tainted(std::forward<T_Rhs>(rhs), internal_tag);
    } else {
      return Tainted(std::forward<T_Rhs>(rhs));
    }
  }

 public:
  Tainted() = default;
  Tainted(const Tainted<T, MC_Sbx>& p) = default;

  Tainted(const SandboxCallback<T>& cb) {
    data = cb.UNSAFE_get();
  }
  
  Tainted(const std::nullptr_t& arg) : data(arg) {
    static_assert(std::is_pointer_v<T>);
  }

  template <typename T_Arg,
            MC_ENABLE_IF(is_fundamental_or_enum_v<T> && is_fundamental_or_enum_v<
                         std::remove_reference_t<T_Arg>>)>
  Tainted(T_Arg&& arg) : data(std::forward<T_Arg>(arg)) {}

  template<typename T_Rhs>
  void assign_raw_pointer(T_Rhs val) {
    static_assert(std::is_pointer_v<T_Rhs>, "Must be a pointer");
    static_assert(std::is_assignable_v<T&, T_Rhs>,
                  "Should assign pointers of compatible types.");
    //TODO(abhishek): check that `val` is a pointer within the sandbox.
    data = val;
  }

  template<typename T_Dummy = void>
  operator bool() const {
    if_constexpr_named(cond1, std::is_pointer_v<T>) {
      return get_raw_value() != nullptr;
    } else {
      auto unknownCase = !(cond1);
      mc_detail_static_fail_because(
          unknownCase,
          "Implicit conversion to bool permitted only for pointers");
    }
  }

  template<typename T2, typename = std::enable_if_t<std::is_pointer_v<T> && std::is_convertible_v<T, const T2*>, T>>
  inline operator Tainted<const T2*, MC_Sbx>() const {
    Tainted<const T2*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data);
    return ret;
  }
  
  template<typename T2, typename = std::enable_if_t<std::is_pointer_v<T> && std::is_convertible_v<T, T2*>, T>>
  inline operator Tainted<T2*, MC_Sbx>() const {
    Tainted<T2*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data);
    return ret;
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

  template<template<typename, typename> typename U1, typename U2, typename U3>
  friend class TaintedBase;
  
  template<typename U1, typename U2>
  friend class Tainted;

private:
  friend class TaintedBase<TaintedVolatile, T, MC_Sbx>;
  
  using T_ClassBase = TaintedBase<TaintedVolatile, T, MC_Sbx>;

  //TODO(abhishek): not picking representation based on MC_Sbx type.
  T data;

  inline auto& get_sandbox_value_ref() noexcept { return data; }
  inline auto& get_sandbox_value_ref() const noexcept { return data; }

  inline std::remove_cv_t<T> get_raw_value() const noexcept {
    return data;
  }

  TaintedVolatile() = default;
  TaintedVolatile(const TaintedVolatile<T, MC_Sbx>& p) = default;

public:
  inline Tainted<const T*, MC_Sbx> operator&() const noexcept {
    auto ref = remove_volatile_from_ptr_cast(&this->get_sandbox_value_ref());
    auto ref_cast = reinterpret_cast<const T*>(ref);
    return Tainted<const T*, MC_Sbx>::internal_factory(ref_cast);
  }

  inline Tainted<T*, MC_Sbx> operator&() noexcept {
    auto taintedVal = &std::as_const(*this);
    auto raw = const_cast<T*>(taintedVal.INTERNAL_unverified_safe());
    return Tainted<T*, MC_Sbx>::internal_factory(raw);
  }

  template<typename T_RhsRef>
  inline TaintedVolatile<T, MC_Sbx>& operator=(T_RhsRef&& val) {
    using T_Rhs = std::remove_reference_t<T_RhsRef>;
    //using T_Rhs_El = std::remove_all_extents_t<T_Rhs>;

    if_constexpr_named(cond1, std::is_same_v<std::remove_const_t<T_Rhs>, std::nullptr_t>) {
      static_assert(std::is_pointer_v<T>,
                    "Null pointer can only be assigned to pointers");
      data = nullptr;
    }
    else if_constexpr_named(cond2,
                            std::is_fundamental_v<T> || std::is_enum_v<T>) {
      get_sandbox_value_ref() = val;
    }
    else {
      // TODO: fail here !
      auto unknownCase = !(cond1 || cond2);
      static_assert(unknownCase, "Unsupported assignment");
    }

    return *this;
  }
};

template <typename T, typename MC_Sbx>
class AppPointer {
  T data_;

 public:
  AppPointer(T data) : data_(data) {}

  operator uintptr_t() {
    return reinterpret_cast<uintptr_t>(data_) & 0xfffffffffff;
  }

  inline auto UNSAFE_unverified() const {
    return data_;
  }

  template <typename T_Func>
  inline auto copy_and_verify(T_Func verifier) const {
    return verifier(data_);
  }
};

template <typename T_Lhs, typename T_Rhs, typename MC_Sbx>
inline Tainted<T_Lhs, MC_Sbx> tainted_static_cast(
    const Tainted<T_Rhs, MC_Sbx>& rhs) noexcept {
  static_assert(std::is_pointer_v<T_Lhs> && std::is_pointer_v<T_Rhs>,
                "tainted_static_cast on incompatible types");

  Tainted<T_Rhs, MC_Sbx> taintedVal = rhs;
  auto raw = static_cast<T_Lhs>(taintedVal.INTERNAL_unverified_safe());
  auto ret = Tainted<T_Lhs, MC_Sbx>::internal_factory(raw);
  return ret;
}

template <typename T_Lhs, typename T_Rhs, typename MC_Sbx>
inline Tainted<T_Lhs, MC_Sbx> tainted_reinterpret_cast(
    const Tainted<T_Rhs, MC_Sbx>& rhs) noexcept {
  static_assert(std::is_pointer_v<T_Lhs> && std::is_pointer_v<T_Rhs>,
                "tainted_reinterpret_cast on incompatible types");

  Tainted<T_Rhs, MC_Sbx> taintedVal = rhs;
  auto raw = reinterpret_cast<T_Lhs>(taintedVal.INTERNAL_unverified_safe());
  auto ret = Tainted<T_Lhs, MC_Sbx>::internal_factory(raw);
  return ret;
}

}  // namespace detail

#if defined(JS_SANDBOX_NOOP)
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxNoop>;

template <typename T>
using AppPointer = detail::AppPointer<T, detail::SandboxNoop>;
#elif defined(JS_SANDBOX_DYLIB)
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxDylib>;

template <typename T>
using AppPointer = detail::AppPointer<T, detail::SandboxDylib>;
#elif defined(JS_SANDBOX_LFI)
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxLFI>;

template <typename T>
using AppPointer = detail::AppPointer<T, detail::SandboxLFI>;
#else
template <typename T>
using Tainted = detail::Tainted<T, detail::SandboxNone>;

template <typename T>
using AppPointer = detail::AppPointer<T, detail::SandboxNone>;
#endif
}

#endif
