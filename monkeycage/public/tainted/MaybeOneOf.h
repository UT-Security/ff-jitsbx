/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * A class storing one of two optional value types that supports in-place lazy
 * construction.
 */

#ifndef mc_tainting_MaybeOneOf_h
#define mc_tainting_MaybeOneOf_h

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"
#include "mozilla/MaybeOneOf.h"

namespace MC {
namespace detail {

template <class T1, class T2, typename MC_Sbx>
class TaintedVolatile<mozilla::MaybeOneOf<T1, T2>, MC_Sbx> {
 private:
  mozilla::MaybeOneOf<T1, T2> data;

 public:
  bool empty() const { return data.empty(); }

  template <class T, class... Args>
  void construct(Args&&... aArgs) {
    data.template construct<T>(std::forward<Args>(aArgs)...);
  }
  
  template <class T>
  TaintedVolatile<T, MC_Sbx>& ref() {
    Tainted<T*, MC_Sbx> ret;
    ret.assign_raw_pointer(&data.template as<T>());
    return *ret;
  }

  template <class T>
  Tainted<const T*, MC_Sbx> ptr() const {
    Tainted<const T*, MC_Sbx> ret;
    ret.assign_raw_pointer(&data.template as<T>());
    return ret;
  }

  template <class T>
  Tainted<T*, MC_Sbx> ptr() {
    Tainted<T*, MC_Sbx> ret;
    ret.assign_raw_pointer(&data.template as<T>());
    return ret;
  }

  template <class T>
  TaintedVolatile<const T, MC_Sbx>& ref() const {
    Tainted<const T*, MC_Sbx> ret;
    ret.assign_raw_pointer(&data.template as<T>());
    return *ret;
  }
  
  template <typename Func>
  constexpr auto mapNonEmpty(Func&& aFunc) const {
    MOZ_ASSERT(!empty());
    if (data.state == mozilla::MaybeOneOf<T1, T2>::SomeT1) {
      return std::forward<Func>(aFunc)(ref<T1>());
    }
    return std::forward<Func>(aFunc)(ref<T2>());
  }

  template <typename Func>
  constexpr auto mapNonEmpty(Func&& aFunc) {
    MOZ_ASSERT(!empty());
    if (data.state == mozilla::MaybeOneOf<T1, T2>::SomeT1) {
      return std::forward<Func>(aFunc)(ref<T1>());
    }
    return std::forward<Func>(aFunc)(ref<T2>());
  }
};

}  // namespace detail
}  // namespace MC
#endif
