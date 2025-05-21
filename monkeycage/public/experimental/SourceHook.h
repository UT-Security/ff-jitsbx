/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_experimental_SourceHook_h
#define mc_experimental_SourceHook_h

#include "js/experimental/SourceHook.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"

namespace js {

inline void SetSourceHook(MCContext* cx, mozilla::UniquePtr<SourceHook> hook) {
  return SetSourceHook(cx->cx_, std::move(hook));
}

/** Remove |cx|'s source hook, and return it. The caller now owns the hook. */
inline mozilla::UniquePtr<SourceHook> ForgetSourceHook(
    MCContext* cx) {
  return ForgetSourceHook(cx->cx_);  
}

}

#endif

#endif
