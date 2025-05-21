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
                                             const js::SliceBudget& budget) {
    return StartIncrementalGC(cx->cx_, options, reason, budget);
}

inline void IncrementalGCSlice(MCContext* cx, GCReason reason,
                                             const js::SliceBudget& budget) {
    return IncrementalGCSlice(cx->cx_, reason, budget);
}

inline bool IncrementalGCHasForegroundWork(MCContext* cx) {
    return IncrementalGCHasForegroundWork(cx->cx_);
}

inline void FinishIncrementalGC(MCContext* cx, GCReason reason) {
    return FinishIncrementalGC(cx->cx_, reason);
}

inline GCSliceCallback SetGCSliceCallback(
    MCContext* cx, MC::Sandbox::Callback<GCSliceCallback> callback) {
  return SetGCSliceCallback(cx->cx_, callback.UNSAFE_get());
}

inline GCNurseryCollectionCallback SetGCNurseryCollectionCallback(
    MCContext* cx, MC::Sandbox::Callback<GCNurseryCollectionCallback> callback) {
 return SetGCNurseryCollectionCallback(cx->cx_, callback.UNSAFE_get()); 
}

inline DoCycleCollectionCallback
SetDoCycleCollectionCallback(MCContext* cx, MC::Sandbox::Callback<DoCycleCollectionCallback> callback) {
    return SetDoCycleCollectionCallback(cx->cx_, callback.UNSAFE_get());
}

inline void SetLowMemoryState(MCContext* cx, bool newState) {
    return SetLowMemoryState(cx->cx_, newState);
}

}

inline bool JS_AddExtraGCRootsTracer(MCContext* cx, MC::Sandbox::Callback<JSTraceDataOp> traceOp,
                                     void* data) { 
 return JS_AddExtraGCRootsTracer(cx->cx_, traceOp.UNSAFE_get(), data);
}

inline void JS_GC(MCContext* cx, JS::GCReason reason = JS::GCReason::API) {
  return JS_GC(cx->cx_, reason);
}

inline void JS_MaybeGC(MCContext* cx) { return JS_MaybeGC(cx->cx_); }

inline void JS_SetGCCallback(MCContext* cx,
                             MC::Sandbox::Callback<JSGCCallback> cb,
                             void* data) {
  return JS_SetGCCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_SetObjectsTenuredCallback(
    MCContext* cx, MC::Sandbox::Callback<JSObjectsTenuredCallback> cb,
    void* data) {
  return JS_SetObjectsTenuredCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline bool JS_AddFinalizeCallback(MCContext* cx, MC::Sandbox::Callback<JSFinalizeCallback> cb, void* data) {
    return JS_AddFinalizeCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_RemoveFinalizeCallback(MCContext* cx, MC::Sandbox::Callback<JSFinalizeCallback> cb) {
    return JS_RemoveFinalizeCallback(cx->cx_, cb.UNSAFE_get());
}

inline bool JS_AddWeakPointerZonesCallback(
    MCContext* cx, MC::Sandbox::Callback<JSWeakPointerZonesCallback> cb, void* data) {
    return JS_AddWeakPointerZonesCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_RemoveWeakPointerZonesCallback(
    MCContext* cx, MC::Sandbox::Callback<JSWeakPointerZonesCallback> cb) {
    return JS_RemoveWeakPointerZonesCallback(cx->cx_, cb.UNSAFE_get());
}

inline bool JS_AddWeakPointerCompartmentCallback(
    MCContext* cx, MC::Sandbox::Callback<JSWeakPointerCompartmentCallback> cb, void* data) {
    return JS_AddWeakPointerCompartmentCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void JS_RemoveWeakPointerCompartmentCallback(
    MCContext* cx, MC::Sandbox::Callback<JSWeakPointerCompartmentCallback> cb) {
    return JS_RemoveWeakPointerCompartmentCallback(cx->cx_, cb.UNSAFE_get());
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

inline void SetHostCleanupFinalizationRegistryCallback(
    MCContext* cx, MC::Sandbox::Callback<JSHostCleanupFinalizationRegistryCallback> cb, void* data) {
 return SetHostCleanupFinalizationRegistryCallback(cx->cx_, cb.UNSAFE_get(), data);
}

inline void ClearKeptObjects(MCContext* cx) { ClearKeptObjects(cx->cx_); }
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

inline void JS_GetGCZealBits(MCContext* cx, uint32_t* zealBits,
                                           uint32_t* frequency,
                                           uint32_t* nextScheduled) {
    return JS_GetGCZealBits(cx->cx_, zealBits, frequency, nextScheduled);
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

#endif

#endif
