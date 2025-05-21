/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Tainted_h
#define mc_Tainted_h

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace MC {

template <typename T>
class AutoStackTainted {
  T inner_;

public:
  template<typename... Args>
  AutoStackTainted(MCContext* cx, Args&&... args) : inner_(cx->cx_, std::forward<Args>(args)...) {}
  
  template<typename... Args>
  AutoStackTainted(Args&&... args) : inner_(std::forward<Args>(args)...) {}
};

}
#else

namespace MC {
template <typename T>
using AutoStackTainted = T; 
}

#endif

#endif
