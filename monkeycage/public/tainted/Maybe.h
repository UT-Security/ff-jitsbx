/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_tainted_Maybe_h
#define mc_tainted_Maybe_h

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"
#include "mozilla/Maybe.h"

namespace MC {
namespace detail {

template <typename T, typename MC_Sbx>
class TaintedVolatile<mozilla::Maybe<T>, MC_Sbx> {
private:
 mozilla::Maybe<T> data;

public:
 /*
   * Constructs a T value in-place in this empty Maybe<T>'s storage. The
   * arguments to |emplace()| are the parameters to T's constructor.
   */
  template <typename... Args>
  constexpr void emplace(MCContext* cx, Args&&... aArgs) {
   data.emplace(MC_UNSAFE(cx), std::forward<Args>(aArgs)...);
  }
  
  template <typename... Args>
  constexpr void emplace(Args&&... aArgs) {
   data.emplace(std::forward<Args>(aArgs)...);
  }

  constexpr Tainted<T*, MC_Sbx> ptr() {
    Tainted<T*, MC_Sbx> ret{nullptr};
    ret.assign_raw_pointer(data.ptr());
    return ret;
  }

  constexpr Tainted<T*, MC_Sbx> operator->() { return ptr(); }
};
}
}

#endif
