/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_WrapperCallbacks_h
#define mc_WrapperCallbacks_h

#include "js/WrapperCallbacks.h"

#ifdef JS_SANDBOX

#include "monkeycage/Sandbox.h"

struct MCWrapObjectCallbacks {
private:
  JSWrapObjectCallbacks inner_;
public:
 explicit MCWrapObjectCallbacks(
     MC::Sandbox::Callback<JSWrapObjectCallback> wrap,
     MC::Sandbox::Callback<JSPreWrapCallback> preWrap)
     : inner_{wrap.UNSAFE_get(), preWrap.UNSAFE_get()} {}

 const JSWrapObjectCallbacks* UNSAFE_get() const { return &inner_; }
};

#else
using MCWrapObjectCallbacks = JSWrapObjectCallbacks;
#endif

#endif
