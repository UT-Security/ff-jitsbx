/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef sandbox_Tainting_h
#define sandbox_Tainting_h

#include <type_traits>
#include <utility>

#include "mozilla/Assertions.h"

#include "sandbox/JitSandbox.h"

namespace js {

template <typename T>
class Untrusted {
  T inner_;

 public:
  Untrusted() {}
  Untrusted(const T& inner) : inner_(inner) {}
  Untrusted(T&& inner) : inner_(std::move(inner)) {}

  Untrusted<T>& operator=(const T& inner) {
    inner_ = inner;
    return *this;
  }
  Untrusted<T>& operator=(T&& inner) {
    inner_ = std::move(inner);
    return *this;
  }

  const T& trust() const& {
    return inner_;
  }
  T& trust() & {
    return inner_;
  }
  T&& trust() && {
    return std::move(inner_);
  }

  template <typename F/*, typename = std::enable_if_t<
    std::is_invocable_r_v<bool, F, const T&>
  >*/>
  const T& verify(
    F&& pred,
    const char* explain = "verify failed on untrusted data"
  ) const& {
    const T& inner(inner_);
    if (!pred(inner)) {
      MOZ_CRASH_UNSAFE(explain);
    }
    return trust();
  }

  template <typename F, typename = std::enable_if_t<
    std::is_invocable_r_v<bool, F, const T&>
  >>
  T& verify(
    F&& pred,
    const char* explain = "verify failed on untrusted data"
  ) & {
    const T& inner(inner_);
    if (!pred(inner)) {
      MOZ_CRASH_UNSAFE(explain);
    }
    return trust();
  }

  template <typename F, typename = std::enable_if_t<
    std::is_invocable_r_v<bool, F, const T&>
  >>
  T&& verify(
    F&& pred,
    const char* explain = "verify failed on untrusted data"
  ) && {
    const T& inner(inner_);
    if (!pred(inner)) {
      MOZ_CRASH_UNSAFE(explain);
    }
    return std::move(*this).trust();
  }
};

template <typename T, typename F/*, typename = std::enable_if_t<
  std::is_invocable_v<F, const T&>
>*/>
auto mapUntrusted(const Untrusted<T>& arg, F&& f) {
  return Untrusted(f(arg.trust()));
}

template <typename T, typename F, typename = std::enable_if_t<
  std::is_invocable_v<F, T&>
>>
auto mapUntrusted(Untrusted<T>& arg, F&& f) {
  return Untrusted(f(arg.trust()));
}

template <typename T, typename F, typename = std::enable_if_t<
  std::is_invocable_v<F, T&&>
>>
auto mapUntrusted(Untrusted<T>&& arg, F&& f) {
  return Untrusted(f(std::move(arg).trust()));
}

template <typename T, typename F, typename = std::enable_if_t<
  std::is_invocable_v<F, const T&>
>>
auto mapUntrusted(const Untrusted<T>* arg, F&& f) {
  return Untrusted(f(arg->trust()));
}

template <typename T, typename F, typename = std::enable_if_t<
  std::is_invocable_v<F, T&>
>>
auto mapUntrusted(Untrusted<T>* arg, F&& f) {
  return Untrusted(f(arg->trust()));
}

}; // namespace js

#endif /* sandbox_Tainting_h */
