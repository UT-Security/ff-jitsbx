/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_JitSandbox_h
#define jit_JitSandbox_h


#include <cstdint>

#include "jit/JitRuntime.h"
#include "js/Stack.h"
#include "threading/ProtectedData.h"
#include "vm/JSContext.h"

namespace js {

namespace jit {

class JitSandboxRuntime {

private:
  WriteOnceData<uint8_t *> sandboxStack_{nullptr};

  WriteOnceData<uintptr_t> sandboxStackLimit_{0};
      
	WriteOnceData<uint8_t *> initialSandboxStackPtr_{nullptr};
      
	WriteOnceData<uintptr_t> addressOfSavedSandboxStackPtr_{0};
      
	WriteOnceData<uintptr_t> addressOfSavedNativeStackPtr_{0};

public:

  JitSandboxRuntime() = default;
  ~JitSandboxRuntime();
  
  [[nodiscard]] bool initialize(JSContext* cx);
      
  void resetStack(JSContext* cx);
  
  uintptr_t addressOfSavedNativeStackPtr() const {
		return addressOfSavedNativeStackPtr_;
	}

	const uint8_t* savedNativeStackPtr() {
		return *(uint8_t **)addressOfSavedNativeStackPtr_.ref();
	}

  void setSavedNativeStackPtr(const uint8_t* ptr) {
    *(uint8_t const **)addressOfSavedNativeStackPtr_.ref() = ptr;      
  }

	uintptr_t addressOfSavedSandboxStackPtr() const {
		return addressOfSavedSandboxStackPtr_;
	}

  const uint8_t* savedSandboxStackPtr() {
    return *(uint8_t **)addressOfSavedSandboxStackPtr_.ref();      
  }

  void setSavedSandboxStackPtr(const uint8_t* ptr) {
    *(uint8_t const **)addressOfSavedSandboxStackPtr_.ref() = ptr;      
  }

  const uintptr_t* addressOfSandboxStackLimit() const {
    return &sandboxStackLimit_.ref();      
  }

  uintptr_t sandboxStackLimit() const {
    return sandboxStackLimit_;      
  }
};

/*class MOZ_RAII JitSandboxContext {

private:
  const JitSandboxRuntime* runtime;         

public:
  explicit JitSandboxContext(JSContext* cx);
  explicit JitSandboxContext(CompileRuntime* rt);
  explicit JitSandboxContext(JitSandboxRuntime* rt);
  ~JitSandboxContext();
      
  uintptr_t addressOfSavedNativeStackPtr() const {
		return runtime->addressOfSavedNativeStackPtr();
	}
  
  uintptr_t addressOfSavedSandboxStackPtr() const {
    return runtime->addressOfSavedSandboxStackPtr();      
  }

  const uintptr_t* addressOfSandboxStackLimit() const {
    return runtime->addressOfSandboxStackLimit();      
  }
};*/

class MOZ_RAII AutoCheckSbxRecursionLimit {
  [[nodiscard]] MOZ_ALWAYS_INLINE bool checkLimitImpl(
      uintptr_t limit, void* sp) const;

 public:
  explicit MOZ_ALWAYS_INLINE AutoCheckSbxRecursionLimit(JSContext* cx) {}
  MOZ_ALWAYS_INLINE ~AutoCheckSbxRecursionLimit() {}

  AutoCheckSbxRecursionLimit(const AutoCheckSbxRecursionLimit&) = delete;
  void operator=(const AutoCheckSbxRecursionLimit&) = delete;

  [[nodiscard]] MOZ_ALWAYS_INLINE bool check(JSContext* cx) const;
  [[nodiscard]] MOZ_ALWAYS_INLINE bool checkDontReport(JSContext* cx) const;
  [[nodiscard]] MOZ_ALWAYS_INLINE bool checkWithExtra(JSContext* cx,
                                                      size_t extra) const;
  [[nodiscard]] MOZ_ALWAYS_INLINE bool checkWithStackPointerDontReport(
      JSContext* cx, void* sp) const;
};


MOZ_ALWAYS_INLINE bool AutoCheckSbxRecursionLimit::checkLimitImpl(
    uintptr_t limit, void* sp) const {
#if JS_STACK_GROWTH_DIRECTION > 0
  return MOZ_LIKELY((uintptr_t)sp < limit);
#else
  return MOZ_LIKELY((uintptr_t)sp > limit);
#endif
}    

MOZ_ALWAYS_INLINE bool AutoCheckSbxRecursionLimit::check(JSContext* cx) const {
  if (MOZ_UNLIKELY(!checkDontReport(cx))) {
    ReportOverRecursed(cx);
    return false;
  }
  return true;
}

MOZ_ALWAYS_INLINE bool AutoCheckSbxRecursionLimit::checkDontReport(
    JSContext* cx) const {
  void* sp = (void*)cx->runtime()->jitSandboxRuntime()->savedSandboxStackPtr();
  return checkWithStackPointerDontReport(cx, sp);
}
    
MOZ_ALWAYS_INLINE bool AutoCheckSbxRecursionLimit::checkWithExtra(
    JSContext* cx, size_t extra) const {
  char* sp = (char*)cx->runtime()->jitSandboxRuntime()->savedSandboxStackPtr();
#if JS_STACK_GROWTH_DIRECTION > 0
  sp += extra;
#else
  sp -= extra;
#endif
  if (MOZ_UNLIKELY(!checkWithStackPointerDontReport(cx, sp))) {
    ReportOverRecursed(cx);
    return false;
  }
  return true;
}

MOZ_ALWAYS_INLINE bool AutoCheckSbxRecursionLimit::checkWithStackPointerDontReport(
    JSContext* cx, void* sp) const {
  uintptr_t sandboxStackLimit = cx->runtime()->jitSandboxRuntime()->sandboxStackLimit();
  if (MOZ_LIKELY(checkLimitImpl(sandboxStackLimit, sp))) {
    return true;
  }
  return false;
}

}  // namespace jit
}  // namespace js

#endif /* jit_JitSandbox_h */
