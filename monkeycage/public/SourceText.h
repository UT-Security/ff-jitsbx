/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_SourceText_h
#define mc_SourceText_h

#include "js/SourceText.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace MC {

namespace detail {

template <typename Unit, typename MC_Sbx>
class TaintedVolatile<JS::SourceText<Unit>, MC_Sbx> {
 private:
  JS::SourceText<Unit> data;

 public:
  inline bool init(MCContext* cx, const Unit* units, size_t unitsLength,
                   JS::SourceOwnership ownership) {
    return data.init(cx->cx_, units, unitsLength, ownership);
  }

  template <typename Char,
            typename = std::enable_if_t<
                std::is_same_v<Char, typename JS::SourceText<Unit>::CharT> &&
                !std::is_same_v<Char, Unit>>>
  inline bool init(MCContext* cx, const Char* chars, size_t charsLength,
                   JS::SourceOwnership ownership) {
    return data.init(cx->cx_, chars, charsLength, ownership);
  }
};
}  // namespace detail
}
#endif
#endif
