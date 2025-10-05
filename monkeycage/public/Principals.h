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

#include "js/Utility.h"
#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"
#include "monkeycage/Tainted.h"

struct MCPrincipals {
  JSPrincipals* inner_;
private:
  static bool writeCb(void* p, JSContext* cx, JSStructuredCloneWriter* writer_) {
    auto principals = static_cast<MCPrincipals*>(p);
    MCContext* mcx = MC_SanitizeContext(cx);
    MC::Tainted<JSStructuredCloneWriter*> writer{nullptr};
    writer.assign_raw_pointer(writer_);
    return principals->write(mcx, writer);
  }

  static bool isSystemOrAddonPrincipalCb(void* p) {
    auto principals = static_cast<MCPrincipals*>(p);
    return principals->isSystemOrAddonPrincipal();
  }

  static const ::sandbox::JSPrincipals::Ops* ops() {
    static ::sandbox::JSPrincipals::Ops inner_ {
      MC::Sandbox::RegisterCallback(writeCb).UNSAFE_get(),
      MC::Sandbox::RegisterCallback(isSystemOrAddonPrincipalCb).UNSAFE_get(),
    };

    return &inner_;
  }

public:
  MCPrincipals() {
    inner_ = js_new<::sandbox::JSPrincipals>(ops(), this);
  }

  ~MCPrincipals() {
    js_free((void*)inner_);
  }

#ifdef JS_DEBUG
  uint32_t getDebugToken() {
    return inner_->debugToken;
  }
#endif
  
  void setDebugToken(uint32_t token) {
#ifdef JS_DEBUG
    inner_->debugToken = token;
#endif
  }

  mozilla::Atomic<int32_t, mozilla::SequentiallyConsistent>& refcount() {
    return inner_->refcount;
  }

  virtual bool write(MCContext* cx, MC::Tainted<JSStructuredCloneWriter*> writer) = 0;

  virtual bool isSystemOrAddonPrincipal() = 0; 
};

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

//TODO(JS_SANDBOX): this should take MCPrincipals*
//extern JS_PUBLIC_API void JS_HoldPrincipals(JSPrincipals* principals);

inline void JS_DropPrincipals(MCContext* cx, JSPrincipals* principals) {
  return JS_DropPrincipals(cx->cx_, principals);  
}

inline void JS_SetSecurityCallbacks(MCContext* cx,
                                    const MCSecurityCallbacks* callbacks) {
  JS_SetSecurityCallbacks(cx->cx_, callbacks->UNSAFE_get());
}

inline const JSSecurityCallbacks* JS_GetSecurityCallbacks(MCContext* cx) {
  return JS_GetSecurityCallbacks(cx->cx_);
}

//TODO(abhishek): Since JSPrincipals* has virtual methods we need a wrapper class here.
inline void JS_SetTrustedPrincipals(MCContext* cx, MCPrincipals* prin) {
  return JS_SetTrustedPrincipals(cx->cx_, prin ? prin->inner_ : nullptr);
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

using MCPrincipals = JSPrincipals;
using MCSecurityCallbacks = JSSecurityCallbacks;

#endif

#endif
