/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * A convenience class that makes it easy to perform the operations of a for-of
 * loop.
 */

#ifndef mc_ForOfIterator_h
#define mc_ForOfIterator_h

#include "js/ForOfIterator.h"

#ifdef JS_SANDBOX

#include "monkeycage/Tainted.h"

namespace MC {

namespace detail {

template <typename MC_Sbx>
class TaintedVolatile<JS::ForOfIterator, MC_Sbx> {
 private:
  JS::ForOfIterator data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  inline bool init(
      JS::Handle<JS::Value> iterable,
      JS::ForOfIterator::NonIterableBehavior nonIterableBehavior =
          JS::ForOfIterator::NonIterableBehavior::ThrowOnNonIterable) {
    return data.init(iterable, nonIterableBehavior);
  }

  inline bool next(JS::MutableHandle<JS::Value> val, MC::Tainted<bool*> done) {
    return data.next(val, done.INTERNAL_unverified_safe());
  }

  inline void closeThrow() { data.closeThrow(); }

  inline bool valueIsIterable() const { return data.valueIsIterable(); }
};
}  // namespace detail
}  // namespace MC
#endif

#endif
