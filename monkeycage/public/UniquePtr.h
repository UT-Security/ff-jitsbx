/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_UniquePtr_h
#define mc_UniquePtr_h

#include "js/UniquePtr.h"

#ifdef JS_SANDBOX

namespace mc {

template <typename T, typename D = mozilla::DefaultDelete<T>>
using AppUniquePtr = mozilla::UniquePtr<T, D>;

}

#else

namespace mc {
  
template <typename T, typename D = JS::DeletePolicy<T>>
using AppUniquePtr = js::UniquePtr<T, D>;

}

#endif

#endif
