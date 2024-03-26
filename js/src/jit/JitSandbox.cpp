/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include <cstdint>
#include <sys/mman.h>

#include "threading/ProtectedData.h"
#include "util/Memory.h"
#include "vm/JSContext.h"

using namespace js;
using namespace js::jit;

static MOZ_THREAD_LOCAL(JitSandboxContext*) TlsJitSandboxContext;

JitSandboxRuntime::~JitSandboxRuntime() {
  js_free(sandboxStack_);
  munmap((void*)addressOfSavedSandboxStackPtr_.ref(), 2 * sizeof(uintptr_t));
}

bool JitSandboxRuntime::initialize(JSContext* cx) {
#if JS_STACK_GROWTH_DIRECTION > 0
  MOZ_ASSERT(cx->nativeStackBase() < cx->jitStackLimit);
  uintptr_t sandboxStackSize = cx->jitStackLimit - cx->nativeStackBase();
#else // stack grows up
  MOZ_ASSERT(cx->nativeStackBase() > cx->jitStackLimit);
  uintptr_t sandboxStackSize = cx->nativeStackBase() - cx->jitStackLimit;
#endif // stack grows down
  MOZ_ASSERT(sandboxStackSize > 0);
  
  sandboxStack_ = cx->pod_calloc<uint8_t>(sandboxStackSize + 4096); 
  if (!sandboxStack_) {
    return false;
  }

  addressOfSavedSandboxStackPtr_ = (uintptr_t)mmap(nullptr, 2 * sizeof(uintptr_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
  if (!addressOfSavedSandboxStackPtr_) {
    return false;
  }

#if JS_STACK_GROWTH_DIRECTION > 0
  setSavedSandboxStackPtr(sandboxStack_);
  sandboxStackLimit_ = (uintptr_t)(savedSandboxStackPtr() + sandboxStackSize);
#else
  setSavedSandboxStackPtr(sandboxStack_ + sandboxStackSize + 4096);
  sandboxStackLimit_ = (uintptr_t)(savedSandboxStackPtr() - sandboxStackSize);
#endif

  initialSandboxStackPtr_ = (uint8_t *)savedSandboxStackPtr();
  
  addressOfSavedNativeStackPtr_ = addressOfSavedSandboxStackPtr_ + sizeof(uintptr_t);
  setSavedNativeStackPtr(nullptr);

  return true;
}

void JitSandboxRuntime::resetStack(JSContext* cx) {
  setSavedSandboxStackPtr(initialSandboxStackPtr_);
}

static JitSandboxContext* CurrentJitSandboxContext() {
  if (!TlsJitSandboxContext.init()) {
    return nullptr;
  }
  return TlsJitSandboxContext.get();
}

void jit::SetJitSandboxContext(JitSandboxContext* ctx) {
  MOZ_ASSERT(!CurrentJitSandboxContext());
  TlsJitSandboxContext.set(ctx);
}

JitSandboxContext* jit::GetJitSandboxContext() {
  MOZ_ASSERT(CurrentJitSandboxContext());
  return CurrentJitSandboxContext();
}

JitSandboxContext* jit::MaybeGetJitSandboxContext() { return CurrentJitSandboxContext(); }

JitSandboxContext::JitSandboxContext(const JitSandboxRuntime* rt)
  : runtime(rt) {
  SetJitSandboxContext(this);
}

JitSandboxContext::~JitSandboxContext() {
  MOZ_ASSERT(TlsJitSandboxContext.get() == this);
  TlsJitSandboxContext.set(nullptr);
}
