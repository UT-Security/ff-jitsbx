/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jitsbx/JitSandboxContext.h"
#include "jitsbx/JitSandbox.h"

using namespace js;
using namespace js::jitsbx;

static MOZ_THREAD_LOCAL(JitSandboxContext*) TlsJitSandboxContext;

static JitSandboxContext* CurrentJitSandboxContext() {
  if (!TlsJitSandboxContext.init()) {
    return nullptr;
  }
  return TlsJitSandboxContext.get();
}

void jitsbx::SetJitSandboxContext(JitSandboxContext* ctx) {
  MOZ_ASSERT(!CurrentJitSandboxContext());
  TlsJitSandboxContext.set(ctx);
}

JitSandboxContext* jitsbx::GetJitSandboxContext() {
  MOZ_ASSERT(CurrentJitSandboxContext());
  return CurrentJitSandboxContext();
}

JitSandboxContext* jitsbx::MaybeGetJitSandboxContext() {
  return CurrentJitSandboxContext();
}

JitSandboxContext::JitSandboxContext(const JitSandbox* jitSbx)
    : jitSandbox(jitSbx) {
  SetJitSandboxContext(this);
}

JitSandboxContext::~JitSandboxContext() {
  MOZ_ASSERT(TlsJitSandboxContext.get() == this);
  TlsJitSandboxContext.set(nullptr);
}
