/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_WeakMapPtr_h
#define mc_WeakMapPtr_h

#include "js/WeakMapPtr.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace MC {
namespace detail {

template <typename K, typename V, typename MC_Sbx>
class TaintedVolatile<JS::WeakMapPtr<K, V>, MC_Sbx> {
 private:
  JS::WeakMapPtr<K, V> data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  bool init(MCContext* cx) { return data.init(cx->cx_); }
  bool initialized() { return data.initialized(); }
  void destroy() { data.destroy(); }
  void trace(MC::Tainted<JSTracer*> tracer) {
    data.trace(tracer.INTERNAL_unverified_safe());
  }

  V lookup(const K& key) { return data.lookup(key); }
  bool put(MCContext* cx, const K& key, const V& value) {
    return data.put(cx->cx_, key, value);
  }
  V removeValue(const K& key) { return data.removeValue(key); }
};

}  // namespace detail
}  // namespace MC

#endif

#endif
