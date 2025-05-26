/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_GCVector_h
#define mc_GCVector_h

#include "js/GCVector.h"
#include "monkeycage/RootingAPI.h"

namespace MC {

template <typename T, size_t MinInlineCapacity = 0,
          typename AllocPolicy = js::TempAllocPolicy>
using GCVector = JS::GCVector<T, MinInlineCapacity, AllocPolicy>;

template <typename T, typename AllocPolicy>
using StackGCVector = JS::StackGCVector<T, AllocPolicy>;

#ifdef JS_SANDBOX
// An automatically rooted GCVector for stack use.
template <typename T>
class RootedVector : public Rooted<MC::StackGCVector<T>> {
  using Vec = MC::StackGCVector<T>;
  using Base = MC::Rooted<Vec>;

 public:
  explicit RootedVector(MCContext* cx) : Base(cx, Vec(cx->cx_)) {}
};
#else
template <typename T>
using RootedVector = JS::RootedVector<T>;
#endif
}
#endif
