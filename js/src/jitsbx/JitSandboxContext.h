/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jitsbx_JitSandboxContext_h
#define jitsbx_JitSandboxContext_h

#include "jstypes.h"
#include "jitsbx/JitSandbox.h"

namespace js {

namespace jitsbx {

class MOZ_RAII JitSandboxContext {
 public:
  const JitSandbox* jitSandbox;

  explicit JitSandboxContext(const JitSandbox* jitSbx);
  ~JitSandboxContext();
};

// Get and set the current JIT Sandbox context.
JitSandboxContext* GetJitSandboxContext();
JitSandboxContext* MaybeGetJitSandboxContext();

void SetJitSandboxContext(JitSandboxContext* ctx);

}  // namespace jitsbx
}  // namespace js

#endif /* jitsbx_JitSandboxContext_h */
