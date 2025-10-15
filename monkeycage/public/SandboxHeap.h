/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxHeap_h
#define mc_SandboxHeap_h

#include "monkeycage/Context.h"
#include "monkeycage/SandboxHelpers.h"
#include "monkeycage/SandboxTraits.h"
#include "monkeycage/Tainted.h"
#include "monkeycage/UniquePtr.h"

#include "js/Utility.h"

namespace MC {
namespace detail {

template <typename T>
class SandboxHeapPtr {
private:
  T* inner_;

public:
  SandboxHeapPtr() : inner_(nullptr) {
    inner_ = js_new<T>();
  }

  template <typename... Args>
  SandboxHeapPtr(Args&&... args) : inner_(nullptr) {
    inner_ = js_new<T>(std::forward<Args>(args)...);
  }

  SandboxHeapPtr(SandboxHeapPtr&& other) : inner_(nullptr) {
    inner_ = other.inner_;
    other.inner_ = nullptr;
  }
  
  SandboxHeapPtr(js::UniquePtr<T>&& other) : inner_(nullptr) {
    inner_ = other.release();
  }

  ~SandboxHeapPtr() {
    js_delete(inner_);
  }
  
  SandboxHeapPtr(const SandboxHeapPtr& aOther) = delete;
  void operator=(const SandboxHeapPtr& aOther) = delete;

  //TODO(abhishek): do we need to do anything special for copy/move-constructor?
  inline T* addr() const {
    return const_cast<T*>(inner_);
  }

  inline T* operator->() const {
    return addr();
  }

  SandboxHeapPtr& operator=(SandboxHeapPtr&& aOther) {
    js_delete(inner_);
    inner_ = aOther.inner_;
    aOther.inner_ = nullptr;
    return *this;
  }
};

template <typename T, typename MC_Sbx>
class Tainted<SandboxHeapPtr<T>, MC_Sbx> {
private:
  SandboxHeapPtr<T> data_;

  template <typename T_Arg>
  static inline mc_remove_wrapper_t<T_Arg> ConstructorConvertArg(T_Arg&& arg) {
    if_constexpr_named(cond1, mc_is_Tainted_v<T_Arg>) {
      return arg.INTERNAL_unverified_safe();
    } else if_constexpr_named(cond2, mc_is_TaintedVolatile_v<std::remove_reference_t<T_Arg>>) {
      return arg.INTERNAL_unverified_safe();
    } else {
      return arg;
    }
  }

public:
  Tainted() = default;
  
  template <typename... Args>
  Tainted(MCContext* cx, Args&&... args) : data_(cx->cx_, ConstructorConvertArg<Args>(std::forward<Args>(args))...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }
  
  template <typename... Args>
  Tainted(MCRuntime* rt, Args&&... args) : data_(rt->rt_, std::forward<Args>(args)...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }
  
  template <typename... Args>
  Tainted(Args&&... args) : data_(ConstructorConvertArg<Args>(std::forward<Args>(args))...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }

  Tainted(Tainted&& other) : data_(std::move(other.data_)) {}  

  Tainted(mc::UniquePtr<T>&& other) : data_(std::move(other.INTERNAL_unverified_safe())) {}

  Tainted& operator=(Tainted&& aOther) {
    data_ = std::move(aOther.data_);
    return *this;
  }

  template<typename T2, typename = std::enable_if_t<std::is_convertible_v<T*, const T2*>, T>>
  inline operator Tainted<const T2*, MC_Sbx>() const {
    Tainted<const T2*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data_.addr());
    return ret;
  }
  
  template<typename T2, typename = std::enable_if_t<std::is_convertible_v<T*, T2*>, T>>
  inline operator Tainted<T2*, MC_Sbx>() const {
    Tainted<T2*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data_.addr());
    return ret;
  }

  inline auto& operator*() {
    Tainted<T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data_.addr());
    return *ret;
  }
  
  inline Tainted<T*, MC_Sbx> operator->() const {
    return operator Tainted<T*, MC_Sbx>();
  }
  
  inline auto UNSAFE_unverified() const { return data_.addr(); }
  
  Tainted(const Tainted& aOther) = delete;
  void operator=(const Tainted& aOther) = delete;
};

}

template <typename T>
using SandboxHeap = Tainted<MC::detail::SandboxHeapPtr<T>>;

}

#endif
