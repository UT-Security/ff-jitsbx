/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Implements (almost always) lock-free atomic operations. The operations here
 * are a subset of that which can be found in C++11's <atomic> header, with a
 * different API to enforce consistent memory ordering constraints.
 *
 * Anyone caught using |volatile| for inter-thread memory safety needs to be
 * sent a copy of this header and the C++11 standard.
 */

#ifndef mc_tainted_Atomics_h
#define mc_tainted_Atomics_h

#include "mozilla/Atomics.h"

#ifdef JS_SANDBOX

#include "monkeycage/Tainted.h"

namespace mozilla {

/**
 * Atomic<T> implementation for pointer types.
 *
 * An atomic compare-and-swap primitive for pointer variables is provided, as
 * are atomic increment and decement operators.  Also provided are the compound
 * assignment operators for addition and subtraction. Atomic swap (via
 * exchange()) is included as well.
 */
template <typename T, MemoryOrdering Order>
class Atomic<MC::Tainted<T*>, Order>
    : public detail::AtomicBase<MC::Tainted<T*>, Order> {
  typedef typename detail::AtomicBase<MC::Tainted<T*>, Order> Base;

 public:
  constexpr Atomic() : Base() {}
  explicit constexpr Atomic(MC::Tainted<T*> aInit) : Base(aInit) {}

  using Base::operator=;

 private:
  Atomic(Atomic& aOther) = delete;
};

}  // namespace mozilla

#endif

#endif
