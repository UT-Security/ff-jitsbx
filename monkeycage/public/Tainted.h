/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Tainted_h
#define monkeycage_Tainted_h

#include <type_traits>
#include <utility>

#include "js/Utility.h"
#include "js/sandbox/lib.h"

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
class Tainted: public TaintedBase<Tainted, T>  {
  // TODO: maybe deny class types like RLBox to force usage of Tainted Rooted/Handle types.

private:
  using T_ClassBase = TaintedBase<Tainted, T>;
  friend class TaintedBase<Tainted, T>;
  friend class TaintedUnchecked<T>;
  
  // TODO: maybe differentiate between sandbox and app representation here. I suspect this is
  // not required since we are a read-only sandbox. However there are subtleties when it comes
  // to classes with virtual functions, function pointers etc. 
  T data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

  inline std::remove_cv_t<T> get_raw_value() const noexcept { return data; }

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

  constexpr Tainted(const std::nullptr_t& arg) : data(arg) {
    static_assert(std::is_pointer_v<T>);
  }

  template<typename T_Rhs>
  void assign_raw_pointer(T_Rhs val) {
    static_assert(std::is_pointer_v<T_Rhs>, "Must be a pointer.");
    static_assert(std::is_assignable_v<T&, T_Rhs>,
                  "Should assign pointers of compatible types.");

    //TODO: verify that val is a pointer within the sandbox.
    data = val;
  }
};

template <typename T>
class TaintedUnchecked {
public:
  template <MONKEYCAGE_ENABLE_IF(std::is_pointer_v<T>)>
  inline auto UNSAFE_unverified() {
    static_assert(sizeof(this) == sizeof(data), "TaintedUnchecked<T> should be transparent");
    return data;
  }
private:
  T data;
};

// TODO: we probably expect T to be a non-pointer type
// that is usually RAII style created on the application stack.

// TODO: allocate with placement new on the sandbox stack instead
// of using heap allocation.
template <typename T>
class AutoStackTainted {
private:
  Tainted<T*> data;
public:
  AutoStackTainted(): data(nullptr) {
    void* memory = sbx_stackpush(sizeof(T));
    T* ptr = memory ? new (memory) T() : nullptr;
    data.assign_raw_pointer(ptr);
  }
  
  template<typename... Args>
  AutoStackTainted(Args&&... args): data(nullptr) {
    void* memory = sbx_stackpush(sizeof(T));
    T* ptr = memory ? new (memory) T(std::forward<Args>(args)...) : nullptr;
    data.assign_raw_pointer(ptr);
  }

  ~AutoStackTainted() {
    if(data.INTERNAL_unverified_safe()) {
      data.INTERNAL_unverified_safe()->~T();
      sbx_stackpop(sizeof(T), (void*)data.INTERNAL_unverified_safe());
    }
  }

  inline operator const Tainted<T*>&() const { return data; }
  inline operator Tainted<T*>() { return data; }

  inline auto UNSAFE_unverified() const { return data.UNSAFE_unverified(); }
};


template <typename T>
class AutoHeapTainted {
private:
  Tainted<T*> data;
public:
  AutoHeapTainted(): data(nullptr) {
    T* ptr = js_new<T>();
    data.assign_raw_pointer(ptr);
  }

  template<typename... Args>
  AutoHeapTainted(Args&&... args): data(nullptr) {
    T* ptr = js_new<T>(std::forward<Args>(args)...);
    data.assign_raw_pointer(ptr);
  }

  ~AutoHeapTainted() {
    js_delete<T>(data.INTERNAL_unverified_safe());
  }

  inline operator const Tainted<T*>&() const { return data; }

  inline auto UNSAFE_unverified() const { return data.UNSAFE_unverified(); }
};

template<typename T>
class TaintedVolatile: TaintedBase<TaintedVolatile, T> {
  
};

}

#endif
