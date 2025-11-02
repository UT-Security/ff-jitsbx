/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_StreamConsumer_h
#define mc_StreamConsumer_h

#include "js/StreamConsumer.h"

#ifdef JS_SANDBOX

#include "js/sandbox/StreamConsumer.h"

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

namespace MC {
namespace detail {

template <typename MC_Sbx>
class Tainted<JS::StreamConsumer*, MC_Sbx> {
  private:
    JS::StreamConsumer* data;

  inline JS::StreamConsumer* get_raw_value() const noexcept {
    return data;
  }
 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  Tainted() = default;
  Tainted(const Tainted<JS::StreamConsumer*, MC_Sbx>& p) = default;

  Tainted(const std::nullptr_t& arg) : data(arg) {
  }
  
  template<typename T_Rhs>
  void assign_raw_pointer(T_Rhs val) {
    static_assert(std::is_pointer_v<T_Rhs>, "Must be a pointer");
    static_assert(std::is_assignable_v<JS::StreamConsumer*&, T_Rhs>,
                  "Should assign pointers of compatible types.");
    //TODO(abhishek): check that `val` is a pointer within the sandbox.
    data = val;
  }

  operator bool() const { return get_raw_value() != nullptr; }

  bool consumeChunk(const uint8_t* begin, size_t length) {
    return js::sandbox::StreamConsumer_consumeChunk(get_raw_value(), begin, length);
  }

  void streamEnd(JS::OptimizedEncodingListener* listener = nullptr) {
    return js::sandbox::StreamConsumer_streamEnd(get_raw_value(), listener);
  }

  void streamError(size_t errorCode) {
    return js::sandbox::StreamConsumer_streamError(get_raw_value(), errorCode);
  }

  void consumeOptimizedEncoding(const uint8_t* begin,
                                        size_t length) {
    return js::sandbox::StreamConsumer_consumeOptimizedEncoding(get_raw_value(), begin, length);
  }

  void noteResponseURLs(const char* maybeUrl,
                                const char* maybeSourceMapUrl) {
    return js::sandbox::StreamConsumer_noteResponseURLs(get_raw_value(), maybeUrl, maybeSourceMapUrl);
  }
};
}
}


namespace JS {

inline void InitConsumeStreamCallback(
    MCContext* cx, MC::SandboxCallback<ConsumeStreamCallback> consume,
    MC::SandboxCallback<ReportStreamErrorCallback> report) {
  return InitConsumeStreamCallback(cx->cx_, consume.UNSAFE_get(),
                                   report.UNSAFE_get());
}

}  // namespace JS

#endif

#endif
