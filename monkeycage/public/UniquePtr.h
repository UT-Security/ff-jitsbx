/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_UniquePtr_h
#define mc_UniquePtr_h

#include "js/UniquePtr.h"

#ifdef JS_SANDBOX

#include "monkeycage/Tainted.h"
#include "monkeycage/Utility.h"

namespace mc {

template <typename T, typename D = mozilla::DefaultDelete<T>>
using AppUniquePtr = mozilla::UniquePtr<T, D>;

}

namespace MC {
namespace detail {

template <typename T, typename MC_Sbx>
class Tainted<js::UniquePtr<T>, MC_Sbx> {
private:
  js::UniquePtr<T> ptr_;

  /*template <typename T_Arg>
  static inline mc_remove_wrapper_t<T_Arg> ConstructorConvertArg(T_Arg&& arg) {
    if_constexpr_named(cond1, mc_is_Tainted_v<T_Arg>) {
      return arg.INTERNAL_unverified_safe();
    } else if_constexpr_named(cond2, mc_is_TaintedVolatile_v<std::remove_reference_t<T_Arg>>) {
      return arg.INTERNAL_unverified_safe();
    } else {
      return arg;
    }
  }*/

 public:
  constexpr Tainted() : ptr_() {}

  explicit Tainted(Tainted<T*, MC_Sbx> aPtr): ptr_(aPtr.INTERNAL_unverified_safe()) {}

  MOZ_IMPLICIT constexpr Tainted(decltype(nullptr)) : Tainted() {}

  Tainted(Tainted&& aOther): ptr_(std::move(aOther.ptr_)) {}

  Tainted& operator=(Tainted&& aOther) {
    ptr_ = std::move(aOther.ptr_);
    return *this;
  }

  Tainted<T*, MC_Sbx> release() {
    Tainted<T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(ptr_.release());
    return ret;
  }

  inline operator Tainted<const T*, MC_Sbx>() const {
    Tainted<const T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(ptr_.get());
    return ret;
  }

  inline operator Tainted<T*, MC_Sbx>() const {
    Tainted<T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(ptr_.get());
    return ret;
  }

  inline auto& operator*() {
    Tainted<T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(ptr_.get());
    return *ret;
  }

  inline Tainted<T*, MC_Sbx> operator->() const {
    return operator Tainted<T*, MC_Sbx>();
  }

  explicit operator bool() const { return ptr_.get() != nullptr; }

  inline auto UNSAFE_unverified() const { return ptr_.get(); }

  inline auto INTERNAL_unverified_safe() { return std::move(ptr_); }

  Tainted(const Tainted& aOther) = delete;
  void operator=(const Tainted& aOther) = delete;
};
}  // namespace detail
}

namespace mc {

template <typename T>
using UniquePtr = MC::Tainted<js::UniquePtr<T>>;

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... aArgs) {
  return UniquePtr<T>(mc_new<T>(std::forward<Args>(aArgs)...));
}

}

#else

namespace mc {
  
template <typename T, typename D = JS::DeletePolicy<T>>
using AppUniquePtr = js::UniquePtr<T, D>;

}

#endif

#endif
