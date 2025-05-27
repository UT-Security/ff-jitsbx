/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_SourceText_h
#define mc_SourceText_h

#include "js/SourceText.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace MC {

template <typename Unit>
class SourceText final {
public:
  JS::SourceText<Unit> inner_;

  using CharT = typename JS::SourceText<Unit>::CharT;
  
  SourceText() = default;
  ~SourceText() = default;

  [[nodiscard]] MOZ_IS_CLASS_INIT inline bool init(MCContext* cx,
                                                   const Unit* units,
                                                   size_t unitsLength,
                                                   JS::SourceOwnership ownership) {
    return inner_.init(cx->cx_, units, unitsLength, ownership);
  }
  
  template <typename Char,
            typename = std::enable_if_t<std::is_same_v<Char, CharT> &&
                                        !std::is_same_v<Char, Unit>>>
  [[nodiscard]] MOZ_IS_CLASS_INIT inline bool init(MCContext* cx, const Char* chars,
                                            size_t charsLength,
                                            JS::SourceOwnership ownership) {
    return inner_.init(cx->cx_, chars, charsLength, ownership);
  }
  
  [[nodiscard]] inline bool init(MCContext* cx,
                          js::UniquePtr<Unit[], JS::FreePolicy> data,
                          size_t dataLength) {
    return inner_.init(cx->cx_, data, dataLength);
  }
  
  template <typename Char,
            typename = std::enable_if_t<std::is_same_v<Char, CharT> &&
                                        !std::is_same_v<Char, Unit>>>
  [[nodiscard]] inline bool init(MCContext* cx,
                          js::UniquePtr<Char[], JS::FreePolicy> data,
                          size_t dataLength) {
    return inner_.init(cx, data, dataLength);
  }
  
  [[nodiscard]] inline bool initMaybeBorrowed(MCContext* cx,
                                       JS::AutoStableStringChars& linearChars) {
    return inner_.initMaybeBorrowed(cx->cx_, linearChars);
  }
  
  const Unit* units() const { return inner_.units(); }
  
  const CharT* get() const { return inner_.get(); }
  
  bool ownsUnits() const { return inner_.ownsUnits(); }
  
  uint32_t length() const { return inner_.length(); }
  
  Unit* takeUnits() {
    return inner_.takeUnits();
  }
  
  CharT* takeChars() { return inner_.takeChars(); }

private:
  SourceText(const SourceText&) = delete;
  void operator=(const SourceText&) = delete;
};
}

template <typename Unit>
inline JS::SourceText<Unit>& MC_UNSAFE(MC::SourceText<Unit>& srcBuf) {
  return srcBuf.inner_;
}
#else
namespace MC {
  
template <typename Unit>
using SourceText = JS::SourceText<Unit>;

}

template <typename Unit>
inline JS::SourceText<Unit>& MC_UNSAFE(MC::SourceText<Unit>& srcBuf) {
  return srcBuf;
}
#endif

#endif
