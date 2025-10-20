/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * High-level interface to the JS garbage collector.
 */

#ifndef mc_GCAPI_h
#define mc_GCAPI_h

#include "js/GCAPI.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"
#include "monkeycage/SandboxStack.h"
#include "monkeycage/Tainted.h"

struct MCExternalStringCallbacks {
 public:
  JSExternalStringCallbacks* inner_;

 private:
  static void finalizeCb(const void* p, char16_t* chars) {
    auto callbacks = static_cast<const MCExternalStringCallbacks*>(p);
    return callbacks->finalize(chars);
  }

  static size_t sizeOfBufferCb(const void* p, const char16_t* chars,
                               mozilla::MallocSizeOf mallocSizeOf) {
    auto callbacks = static_cast<const MCExternalStringCallbacks*>(p);
    return callbacks->sizeOfBuffer(chars, mallocSizeOf);
  }

  static const sandbox::JSExternalStringCallbacks::Ops* ops() {
    static const sandbox::JSExternalStringCallbacks::Ops inner_{
        MC::Sandbox::RegisterCallback(finalizeCb).UNSAFE_get(),
        MC::Sandbox::RegisterCallback(sizeOfBufferCb).UNSAFE_get(),
    };

    return &inner_;
  }

 public:
  MCExternalStringCallbacks() {
    inner_ = js_new<sandbox::JSExternalStringCallbacks>(ops(), (const void*)this);
  }

  ~MCExternalStringCallbacks() { js_free((void*)inner_); }

  virtual void finalize(char16_t* chars) const = 0;
  virtual size_t sizeOfBuffer(const char16_t* chars,
                              mozilla::MallocSizeOf mallocSizeOf) const = 0;
};

inline JSString* JS_NewExternalString(
    MCContext* cx, const char16_t* chars, size_t length,
    const MCExternalStringCallbacks* callbacks) {
  return JS_NewExternalString(cx->cx_, chars, length, callbacks->inner_);
}

inline JSString* JS_NewMaybeExternalString(
    MCContext* cx, const char16_t* chars, size_t length,
    const MCExternalStringCallbacks* callbacks,
    MC::Tainted<bool*> allocatedExternal) {
  return JS_NewMaybeExternalString(
      cx->cx_, chars, length, callbacks->inner_,
      allocatedExternal.INTERNAL_unverified_safe());
}

inline const MCExternalStringCallbacks* MC_GetExternalStringCallbacks(
    JSString* str) {
  return static_cast<const MCExternalStringCallbacks*>(
      static_cast<const sandbox::JSExternalStringCallbacks*>(
          JS_GetExternalStringCallbacks(str))
          ->getExternalStringCallbacks());
}

namespace JS {

inline void PrepareZoneForGC(MCContext* cx, Zone* zone) {
    return PrepareZoneForGC(cx->cx_, zone);    
}

inline void PrepareForFullGC(MCContext* cx) {
    return PrepareForFullGC(cx->cx_);
}

inline void PrepareForIncrementalGC(MCContext* cx) {
    return PrepareForIncrementalGC(cx->cx_);
}

inline void NonIncrementalGC(MCContext* cx, JS::GCOptions options,
                                           GCReason reason) {
    return NonIncrementalGC(cx->cx_, options, reason);
}

inline void StartIncrementalGC(MCContext* cx,
                                             JS::GCOptions options,
                                             GCReason reason,
                                             MC::Tainted<const js::SliceBudget*> budget) {
    return StartIncrementalGC(cx->cx_, options, reason, *budget.INTERNAL_unverified_safe());
}

inline void IncrementalGCSlice(MCContext* cx, GCReason reason,
                                             MC::Tainted<const js::SliceBudget*> budget) {
    return IncrementalGCSlice(cx->cx_, reason, *budget.INTERNAL_unverified_safe());
}

inline bool IncrementalGCHasForegroundWork(MCContext* cx) {
    return IncrementalGCHasForegroundWork(cx->cx_);
}

inline void FinishIncrementalGC(MCContext* cx, GCReason reason) {
    return FinishIncrementalGC(cx->cx_, reason);
}

inline void AbortIncrementalGC(MCContext* cx) {
  return AbortIncrementalGC(cx->cx_);
}

inline MC::SandboxCallback<GCSliceCallback> SetGCSliceCallback(
    MCContext* cx, MC::SandboxCallback<GCSliceCallback> callback) {
  GCSliceCallback UNSAFE_callback = SetGCSliceCallback(cx->cx_, callback.UNSAFE_get());
  return MC::Sandbox::RetrieveCallback(UNSAFE_callback);
}

inline MC::SandboxCallback<GCNurseryCollectionCallback> SetGCNurseryCollectionCallback(
    MCContext* cx, MC::SandboxCallback<GCNurseryCollectionCallback> callback) {
 GCNurseryCollectionCallback UNSAFE_callback = SetGCNurseryCollectionCallback(cx->cx_, callback.UNSAFE_get()); 
 return MC::Sandbox::RetrieveCallback(UNSAFE_callback);
}

inline MC::SandboxCallback<DoCycleCollectionCallback>
SetDoCycleCollectionCallback(MCContext* cx, MC::SandboxCallback<DoCycleCollectionCallback> callback) {
    DoCycleCollectionCallback UNSAFE_callback = SetDoCycleCollectionCallback(cx->cx_, callback.UNSAFE_get());
    return MC::Sandbox::RetrieveCallback(UNSAFE_callback);
}

inline void SetCreateGCSliceBudgetCallback(
    MCContext* cx, MC::SandboxCallback<CreateSliceBudgetCallback> cb) {
  return SetCreateGCSliceBudgetCallback(cx->cx_, cb.UNSAFE_get());
}

inline bool IsIncrementalGCInProgress(MCContext* cx) {
    return IsIncrementalGCInProgress(cx->cx_);
}

inline bool IsIncrementalGCInProgress(MCRuntime* rt) {
    return IsIncrementalGCInProgress(rt->rt_);
}

inline bool WasIncrementalGC(MCRuntime* rt) {
    return WasIncrementalGC(rt->rt_);
}

inline void SetLowMemoryState(MCContext* cx, bool newState) {
    return SetLowMemoryState(cx->cx_, newState);
}

}

namespace MC {

#ifdef DEBUG
using AutoAssertNoGC = SandboxStack<JS::AutoAssertNoGC>;
using AutoSuppressGCAnalysis = SandboxStack<JS::AutoSuppressGCAnalysis>;
using AutoAssertGCCallback = SandboxStack<JS::AutoAssertGCCallback>;
using AutoCheckCannotGC = SandboxStack<JS::AutoCheckCannotGC>;
#else
using AutoAssertNoGC = JS::AutoAssertNoGC;
using AutoSuppressGCAnalysis = JS::AutoSuppressGCAnalysis;
using AutoAssertGCCallback = JS::AutoAssertGCCallback;
using AutoCheckCannotGC = JS::AutoCheckCannotGC;
#endif

}

inline bool JS_AddExtraGCRootsTracer(MCContext* cx, MC::SandboxCallback<JSTraceDataOp> traceOp,
                                     void* data) { 
 return JS_AddExtraGCRootsTracer(cx->cx_, traceOp.UNSAFE_get(), data);
}

inline void JS_GC(MCContext* cx, JS::GCReason reason = JS::GCReason::API) {
  return JS_GC(cx->cx_, reason);
}

inline void JS_MaybeGC(MCContext* cx) { return JS_MaybeGC(cx->cx_); }

inline void JS_SetGCCallback(MCContext* cx,
                             MC::SandboxCallback<JSGCCallback> cb,
                             void* data) {
  return JS_SetGCCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_SetObjectsTenuredCallback(
    MCContext* cx, MC::SandboxCallback<JSObjectsTenuredCallback> cb,
    void* data) {
  return JS_SetObjectsTenuredCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline bool JS_AddFinalizeCallback(MCContext* cx, MC::SandboxCallback<JSFinalizeCallback> cb, void* data) {
    return JS_AddFinalizeCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_RemoveFinalizeCallback(MCContext* cx, MC::SandboxCallback<JSFinalizeCallback> cb) {
    return JS_RemoveFinalizeCallback(cx->cx_, cb.UNSAFE_get());
}

inline bool JS_AddWeakPointerZonesCallback(
    MCContext* cx, MC::SandboxCallback<JSWeakPointerZonesCallback> cb, void* data) {
    return JS_AddWeakPointerZonesCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_RemoveWeakPointerZonesCallback(
    MCContext* cx, MC::SandboxCallback<JSWeakPointerZonesCallback> cb) {
    return JS_RemoveWeakPointerZonesCallback(cx->cx_, cb.UNSAFE_get());
}

inline bool JS_AddWeakPointerCompartmentCallback(
    MCContext* cx, MC::SandboxCallback<JSWeakPointerCompartmentCallback> cb, void* data) {
    return JS_AddWeakPointerCompartmentCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_RemoveWeakPointerCompartmentCallback(
    MCContext* cx, MC::SandboxCallback<JSWeakPointerCompartmentCallback> cb) {
    return JS_RemoveWeakPointerCompartmentCallback(cx->cx_, cb.UNSAFE_get());
}

inline bool JS_UpdateWeakPointerAfterGC(MC::Tainted<JSTracer*> trc,
                                        MC::Heap<JSObject*>* objp) {
    MOZ_ASSERT(objp);
    JSObject** raw_objp = objp->unsafeGet();
    MC::SandboxStack<JSObject*> sbx_objp(*raw_objp);
    bool ret = JS_UpdateWeakPointerAfterGCUnbarriered(
        trc.INTERNAL_unverified_safe(), sbx_objp.UNSAFE_unverified());
    if (*sbx_objp.UNSAFE_unverified() != *raw_objp) {
      *raw_objp = *sbx_objp.UNSAFE_unverified();
    }
    return ret;
}

inline bool JS_UpdateWeakPointerAfterGCUnbarriered(MC::Tainted<JSTracer*> trc,
                                                   MC::Tainted<JSObject**> objp) {
  return JS_UpdateWeakPointerAfterGCUnbarriered(trc.INTERNAL_unverified_safe(),
                                                objp.INTERNAL_unverified_safe());
}

inline bool JS_UpdateWeakPointerAfterGCUnbarriered(MC::Tainted<JSTracer*> trc,
                                                   JSObject** objp) {
    MOZ_ASSERT(objp);
    MC::SandboxStack<JSObject*> sbx_objp(*objp);
    bool ret = JS_UpdateWeakPointerAfterGCUnbarriered(trc, sbx_objp);
    if (*sbx_objp.UNSAFE_unverified() != *objp) {
        *objp = *sbx_objp.UNSAFE_unverified();
    }
    return ret;
}

inline void JS_SetGCParameter(MCContext* cx, JSGCParamKey key, uint32_t value) {
  return JS_SetGCParameter(cx->cx_, key, value);
}

inline void JS_ResetGCParameter(MCContext* cx, JSGCParamKey key) {
  return JS_ResetGCParameter(cx->cx_, key);
}

inline uint32_t JS_GetGCParameter(MCContext* cx, JSGCParamKey key) {
  return JS_GetGCParameter(cx->cx_, key);
}

namespace JS {

inline GCReason WantEagerMinorGC(MCRuntime* rt) {
    return WantEagerMinorGC(rt->rt_);
}

inline GCReason WantEagerMajorGC(MCRuntime* rt) {
    return WantEagerMajorGC(rt->rt_);
}

inline void MaybeRunNurseryCollection(MCRuntime* rt, JS::GCReason reason) {
    return MaybeRunNurseryCollection(rt->rt_, reason);
}

inline void SetHostCleanupFinalizationRegistryCallback(
    MCContext* cx, MC::SandboxCallback<JSHostCleanupFinalizationRegistryCallback> cb, void* data) {
 return SetHostCleanupFinalizationRegistryCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void ClearKeptObjects(MCContext* cx) { ClearKeptObjects(cx->cx_); }

inline bool AtomsZoneIsCollecting(MCRuntime* runtime) {
    return AtomsZoneIsCollecting(runtime->rt_);    
}

}  // namespace JS

namespace js {
namespace gc {

inline JSObject* NewMemoryInfoObject(MCContext* cx) {
  return NewMemoryInfoObject(cx->cx_);
}

inline void FinalizeDeadNurseryObject(MCContext* cx, JSObject* obj) {
  return FinalizeDeadNurseryObject(cx->cx_, obj);
}
}  // namespace gc
}  // namespace js

#  ifdef JS_GC_ZEAL

inline void JS_GetGCZealBits(MCContext* cx, MC::Tainted<uint32_t*> zealBits,
                                           MC::Tainted<uint32_t*> frequency,
                                           MC::Tainted<uint32_t*> nextScheduled) {
  return JS_GetGCZealBits(cx->cx_, zealBits.INTERNAL_unverified_safe(),
                          frequency.INTERNAL_unverified_safe(),
                          nextScheduled.INTERNAL_unverified_safe());
}

inline void JS_SetGCZeal(MCContext* cx, uint8_t zeal,
                                       uint32_t frequency) {
    return JS_SetGCZeal(cx->cx_, zeal, frequency);
}

inline void JS_UnsetGCZeal(MCContext* cx, uint8_t zeal) {
    return JS_UnsetGCZeal(cx->cx_, zeal);
}

inline void JS_ScheduleGC(MCContext* cx, uint32_t count) {
    return JS_ScheduleGC(cx->cx_, count);
}

#endif

#else

using MCExternalStringCallbacks = JSExternalStringCallbacks;

#endif

#endif
