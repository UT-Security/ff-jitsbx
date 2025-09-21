/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxStack_h
#define mc_SandboxStack_h

#include "monkeycage/unsafe/SandboxImpl.h"
#include "monkeycage/Context.h"
#include "monkeycage/SandboxHelpers.h"
#include "monkeycage/SandboxTraits.h"
#include "monkeycage/Tainted.h"

#include "js/Utility.h"

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

class SandboxStackBytes {
  private:
    void* bytes_;
    size_t len_;

  public:
    SandboxStackBytes(size_t len): len_(len) {
      bytes_ = js_malloc(len);
    }

    ~SandboxStackBytes() {
      if (bytes_) {
        js_free(bytes_);
      }
    }
  
    void* begin() { return bytes_; }
    size_t size() { return len_; }
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
#elif defined(JS_SANDBOX_LFI)
template <typename T>
class SandboxStackPtr {
private:
  T* inner_;

public:
  SandboxStackPtr() : inner_(nullptr) {
    void* memory = monkeycage_stackpush(sizeof(T));
    inner_ = memory ? new (memory) T() : nullptr;
  }

  template <typename... Args>
  SandboxStackPtr(Args&&... args) : inner_(nullptr) {
    void* memory = monkeycage_stackpush(sizeof(T));
    inner_ = memory ? new (memory) T(std::forward<Args>(args)...) : nullptr;
  }

  ~SandboxStackPtr() {
    if (inner_) {
      inner_->~T();
      monkeycage_stackpop(sizeof(T), (void*)inner_);
    }
  }

  //TODO(abhishek): do we need to do anything special for copy/move-constructor?
  inline T* addr() const {
    return const_cast<T*>(inner_);
  }

  inline T* operator->() const {
    return addr();
  }
};


class SandboxStackBytes {
private:
  void* bytes_;
  size_t len_;

public:
  SandboxStackBytes(size_t len) : len_(len) {
    void* bytes_ = monkeycage_stackpush(sizeof(T));
  }

  ~SandboxStackBytes() {
    if (bytes_) {
      monkeycage_stackpop(sizeof(T), bytes_);
    }
  }

  void* begin() { return bytes_; }
  size_t size() { return len_; }
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
  Tainted(MCContext* cx, Args&&... args) : data_(MC_UNSAFE(cx), ConstructorConvertArg<Args>(std::forward<Args>(args))...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }
  
  template <typename... Args>
  Tainted(MCRuntime* rt, Args&&... args) : data_(MC_UNSAFE(rt), std::forward<Args>(args)...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }
  
  template <typename... Args>
  Tainted(Args&&... args) : data_(ConstructorConvertArg<Args>(std::forward<Args>(args))...) {
    //TODO(abhishek): test that data_ is valid pointer within sandbox memory.
  }

  inline operator Tainted<const T*, MC_Sbx>() const {
    Tainted<const T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data_.addr());
    return ret;
  }
  
  inline operator Tainted<T*, MC_Sbx>() const {
    Tainted<T*, MC_Sbx> ret{nullptr};
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
};

}

template <typename T>
using SandboxStack = Tainted<MC::detail::SandboxStackPtr<T>>;

using SandboxStackBytes = MC::detail::SandboxStackBytes;

}

#endif
