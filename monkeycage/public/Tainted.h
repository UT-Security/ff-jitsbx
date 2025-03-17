/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Tainted_h
#define monkeycage_Tainted_h

#include <type_traits>
#include <utility>

namespace monkeycage {

namespace detail {

#define MONKEYCAGE_ENABLE_IF(...) std::enable_if_t<__VA_ARGS__>* = nullptr  

}

template<typename T>
class TaintedUnchecked;

template<typename T>
class Tainted;

template<typename T>
class TaintedVolatile;

template<template<typename> typename T_Wrap, typename T>
class TaintedBase {

public:
 inline auto& impl() { return *static_cast<T_Wrap<T>*>(this); }
 inline auto& impl() const { return *static_cast<const T_Wrap<T>*>(this); }

 inline auto UNSAFE_unverified() const { return impl().get_raw_value(); }

 inline auto INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

private:
  using T_OpDerefRet = TaintedVolatile<std::remove_pointer_t<T>>;

public:
  inline T_OpDerefRet& operator*() const {
    static_assert(std::is_pointer_v<T>, "Operator * only allowed on pointers");
    auto ret_ptr_const = reinterpret_cast<const T_OpDerefRet*>(impl().get_raw_value());

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

template<typename T>
class Tainted: TaintedBase<Tainted, T>  {
  // TODO: maybe deny class types like RLBox to force usage of Tainted Rooted/Handle types.

private:
  using T_ClassBase = TaintedBase<Tainted, T>;
  
  // TODO: maybe differentiate between sandbox and app representation here. I suspect this is
  // not required since we are a read-only sandbox. However there are subtleties when it comes
  // to classes with virtual functions, function pointers etc. 
  T data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

  inline std::remove_cv_t<T> get_raw_value() { return data; }

  template <typename T2 = T, MONKEYCAGE_ENABLE_IF(std::is_pointer_v<T2>)>
  Tainted(T2 val, const void* /* internal tag */) : data(val) {
    // Sanity check
    static_assert(std::is_pointer_v<T>);
  }

  template<typename T_Rhs>
  static inline Tainted<T> internal_factory(T_Rhs&& rhs) {
    if constexpr (std::is_pointer_v<std::remove_reference_t<T_Rhs>>) {
      const void* internal_tag = nullptr;
      return Tainted(std::forward<T_Rhs>(rhs), internal_tag);
    } else {
      return Tainted(std::forward<T_Rhs>(rhs));
    }
  }

public:
  Tainted() = default;
  Tainted(const Tainted<T>& p) = default;

  Tainted(const std::nullptr_t& arg) : data(arg) {
    static_assert(std::is_pointer_v<T>);
  }
};

template <typename T>
class TaintedUnchecked {
public:
  template <MONKEYCAGE_ENABLE_IF(std::is_pointer_v<T>)>
  inline auto to_checked() {
    // TODO: use the actual secure interface that checks the pointer
    return Tainted<T>::internal_factory(data);
  }
private:
  friend class Tainted<T>;
  T data;
};

template<typename T>
class TaintedVolatile: TaintedBase<TaintedVolatile, T> {
  
};

}

#endif
