/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SliceBudget_h
#define mc_SliceBudget_h

#include "js/SliceBudget.h"

#ifdef JS_SANDBOX

#include "monkeycage/Tainted.h"

namespace MC {
namespace detail {

template <typename MC_Sbx>
class Tainted<js::SliceBudget, MC_Sbx> {
 private:
  js::SliceBudget data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }

  Tainted(const TaintedVolatile<js::SliceBudget, MC_Sbx>& p) : data(p.INTERNAL_unverified_safe()) {}
};

template <typename MC_Sbx>
class TaintedVolatile<const js::SliceBudget, MC_Sbx> {
 private:
  const js::SliceBudget data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  bool isWorkBudget() const { return data.isWorkBudget(); }
  bool isTimeBudget() const { return data.isTimeBudget(); }
  bool isUnlimited() const { return data.isUnlimited(); }

  int64_t timeBudget() const { return data.timeBudget(); }
  int64_t workBudget() const { return data.workBudget(); }
};

template <typename MC_Sbx>
class TaintedVolatile<js::SliceBudget, MC_Sbx> {
 private:
  js::SliceBudget data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  TaintedVolatile<bool, MC_Sbx>& idle() {
    return *Tainted<bool*, MC_Sbx>::internal_factory(&data.idle);
  }
  TaintedVolatile<bool, MC_Sbx>& extended() {
    return *Tainted<bool*, MC_Sbx>::internal_factory(&data.extended);
  }

  bool isWorkBudget() const { return data.isWorkBudget(); }
  bool isTimeBudget() const { return data.isTimeBudget(); }
  bool isUnlimited() const { return data.isUnlimited(); }

  int64_t timeBudget() const { return data.timeBudget(); }
  int64_t workBudget() const { return data.workBudget(); }
};
}  // namespace detail

}  // namespace MC

#endif

#endif
