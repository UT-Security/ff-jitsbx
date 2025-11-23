/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_AutoWritableJitCode_h
#define jit_AutoWritableJitCode_h

#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/ScopeExit.h"
#include "mozilla/TimeStamp.h"

#include <stddef.h>

#include "jit/ExecutableAllocator.h"
#include "jit/JitCode.h"
#include "jit/ProcessExecutableMemory.h"
#include "vm/JSContext.h"
#include "vm/Realm.h"
#include "vm/Runtime.h"

namespace js::jit {

// This class ensures JIT code is executable on its destruction. Creators
// must call makeWritable(), and not attempt to write to the buffer if it fails.
//
// AutoWritableJitCodeFallible may only fail to make code writable; it cannot
// fail to make JIT code executable (because the creating code has no chance to
// recover from a failed destructor).
class MOZ_RAII AutoWritableJitCodeFallible {
  JitCode* code_;

  JSRuntime* runtime() { return code_->runtimeFromMainThread(); }

  void* addr() const { return code_->executable_.xStart; }
  size_t size() const { return code_->executable_.desc.xSize; }

  void* dataAddr() const { return code_->executable_.rwStart; }
  size_t dataSize() const { return code_->executable_.desc.rwSize; }

 public:
  explicit AutoWritableJitCodeFallible(JitCode* code) : code_(code) {
    runtime()->toggleAutoWritableJitCodeActive(true);
  }

  [[nodiscard]] bool makeWritable() {
#ifndef JS_SANDBOX_LFI
    return ExecutableAllocator::makeWritable(addr(), size());
#else
    return true;
#endif
  }

  ~AutoWritableJitCodeFallible() {
    mozilla::TimeStamp startTime = mozilla::TimeStamp::Now();
    auto timer = mozilla::MakeScopeExit([&] {
      if (Realm* realm = runtime()->mainContextFromOwnThread()->realm()) {
        realm->timers.protectTime += mozilla::TimeStamp::Now() - startTime;
      }
    });

#ifndef JS_SANDBOX_LFI
    if (!ExecutableAllocator::makeExecutableAndFlushICache(addr(), size())) {
      MOZ_CRASH();
    }
#endif
    runtime()->toggleAutoWritableJitCodeActive(false);
  }
};

// Infallible variant of AutoWritableJitCodeFallible, ensures writable during
// construction
class MOZ_RAII AutoWritableJitCode : private AutoWritableJitCodeFallible {
 public:
  explicit AutoWritableJitCode(JitCode* code)
      : AutoWritableJitCodeFallible(code) {
    AutoEnterOOMUnsafeRegion oomUnsafe;
    if (!makeWritable()) {
      oomUnsafe.crash("Failed to mmap. Likely no mappings available.");
    }
  }
};

}  // namespace js::jit

#endif /* jit_AutoWritableJitCode_h */
