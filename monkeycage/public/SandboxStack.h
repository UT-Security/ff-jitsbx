/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxImpl_h
#define mc_SandboxImpl_h

#include "monkeycage/unsafe/SandboxImpl.h"
#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace MC {
namespace detail {
#if defined(JS_SANDBOX_NOOP)
template <typename T>
class SandboxStackPtr {
 private:
  T inner_;

 public:
  SandboxStackPtr() = default;

  template <typename... Args>
  SandboxStackPtr(Args&&... args) : inner_(std::forward<Args>(args)...) {}

  // TODO(abhishek): do we need to do anything special for
  // copy/move-constructor?
  inline T* addr() const { return const_cast<T*>(&inner_); }

  inline T* operator->() const { return addr(); }
};
#elif defined(JS_SANDBOX_DYLIB)
template <typename T>
class SandboxStackPtr {
private:
  T inner_;

public:
  SandboxStackPtr() = default;

  template <typename... Args>
  SandboxStackPtr(Args&&... args) : inner_(std::forward<Args>(args)...) {}

  //TODO(abhishek): do we need to do anything special for copy/move-constructor?
  inline T* addr() const {
    return const_cast<T*>(&inner_);
  }

  inline T* operator->() const {
    return addr();
  }
};
#else
template <typename T>
class SandboxStackPtr {
private:
  T inner_;

public:
  SandboxStackPtr() = default;

  template <typename... Args>
  SandboxStackPtr(Args&&... args) : inner_(std::forward<Args>(args)...) {}

  //TODO(abhishek): do we need to do anything special for copy/move-constructor?

  inline T* addr() const {
    return const_cast<T*>(&inner_);
  }

  inline T* operator->() const {
    return addr();
  }
};
#endif

template <typename T, typename MC_Sbx>
class Tainted<SandboxStackPtr<T>, MC_Sbx> {
private:
  SandboxStackPtr<T> data_;

public:
  Tainted() = default;
  
  template <typename... Args>
  Tainted(MCContext* cx, Args&&... args) : data_(MC_UNSAFE(cx), std::forward<Args>(args)...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }
  
  template <typename... Args>
  Tainted(Args&&... args) : data_(std::forward<Args>(args)...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }

  //TODO(abhishek): revisit the type-parameter T to TaintedVolatile.
  using T_OpDerefRet = TaintedVolatile<T, MC_Sbx>;

  inline T_OpDerefRet& operator*() {
    auto ret_ptr_const =
      reinterpret_cast<const T_OpDerefRet*>(data_.addr());
    // Safe - If T_OpDerefRet is not a const ptr, this is trivially safe
    //        If T_OpDerefRet is a const ptr, then the const is captured
    //        inside the wrapper
    auto ret_ptr = const_cast<T_OpDerefRet*>(ret_ptr_const);
    return *ret_ptr;
  }
  
  inline const T_OpDerefRet* operator->() const {
    return reinterpret_cast<const T_OpDerefRet*>(data_.addr());
  }

  inline T_OpDerefRet* operator->() {
    return const_cast<T_OpDerefRet*>(std::as_const(*this).operator->());
  }
};

}

template <typename T>
using SandboxStack = Tainted<MC::detail::SandboxStackPtr<T>>;

}

#endif
