/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/** GC-safe representations of consecutive JS::Value in memory. */

#ifndef mc_ValueArray_h
#define mc_ValueArray_h

#include "js/ValueArray.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/TypeDecls.h"
#include "monkeycage/GCVector.h"

namespace MC {
  
template <size_t N>
using RootedValueArray = Rooted<JS::ValueArray<N>>;

}

namespace JS {
  
inline HandleValueArray::HandleValueArray(const MC::RootedVector<Value>& values)
      : length_(values.length()), elements_(values.begin()) {}

template <size_t N>
inline HandleValueArray::HandleValueArray(const MC::RootedValueArray<N>& values)
      : length_(N), elements_(values.begin()) {}

}

#else

namespace MC {
template <size_t N>
using RootedValueArray = JS::RootedValueArray<N>;
}

#endif

#endif
