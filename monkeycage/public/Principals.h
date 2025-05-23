/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JSPrincipals and related interfaces. */

#ifndef mc_Principals_h
#define mc_Principals_h

#include "js/Principals.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

struct MCSecurityCallbacks {
private:
  JSSecurityCallbacks inner_;
public:
 explicit MCSecurityCallbacks(
     MC::SandboxCallback<JSCSPEvalChecker> contentSecurityPolicyAllows,
     MC::SandboxCallback<JSSubsumesOp> subsumes)
     : inner_{contentSecurityPolicyAllows.UNSAFE_get(), subsumes.UNSAFE_get()} {}
  const JSSecurityCallbacks* UNSAFE_get() const { return &inner_; }
};

inline void JS_SetSecurityCallbacks(
    MCContext* cx, const MCSecurityCallbacks* callbacks) {
  JS_SetSecurityCallbacks(cx->cx_, callbacks->UNSAFE_get());
}

inline const JSSecurityCallbacks* JS_GetSecurityCallbacks(MCContext* cx) {
  return JS_GetSecurityCallbacks(cx->cx_);
}

//TODO(abhishek): Since JSPrincipals* has virtual methods we need a wrapper class here.
inline void JS_SetTrustedPrincipals(MCContext* cx, JSPrincipals* prin) {
  return JS_SetTrustedPrincipals(cx->cx_, prin);
}

inline void JS_InitDestroyPrincipalsCallback(
    MCContext* cx,
    MC::SandboxCallback<JSDestroyPrincipalsOp> destroyPrincipals) {
  return JS_InitDestroyPrincipalsCallback(cx->cx_,
                                         destroyPrincipals.UNSAFE_get());
}

inline void JS_InitReadPrincipalsCallback(
    MCContext* cx, MC::SandboxCallback<JSReadPrincipalsOp> read) {
  return JS_InitReadPrincipalsCallback(cx->cx_, read.UNSAFE_get());
}
#else

using MCSecurityCallbacks = JSSecurityCallbacks;

#endif

#endif
