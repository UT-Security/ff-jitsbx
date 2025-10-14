/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "monkeycage/Context.h"

#ifdef JS_SANDBOX 

#include "js/sandbox/RootingAPI.h"
#include "js/TracingAPI.h"
#include "mcapi.h"
#include "monkeycage/Sandbox.h"
#include "monkeycage/StoreBuffer.h"

static inline void TraceExactStackRootTraceableList(JSTracer* trc,
                                                    mc::StackRootedBase* listHead,
                                                    const char* name) {
  for (mc::StackRootedBase* root = listHead; root; root = root->previous()) {
    static_cast<mc::StackRootedTraceableBase*>(root)->trace(trc, name);
  }
}

static inline void TraceStackRoots(JSTracer* trc,
                                   MC::RootedListHeads& stackRoots) {
  JS::TraceExactStackRootLists(trc, stackRoots);

  // RootedTraceable uses virtual dispatch.
  MC::AutoSuppressGCAnalysis nogc;

  TraceExactStackRootTraceableList(trc, stackRoots[JS::RootKind::Traceable],
                                   "Traceable");
}

void MC::RootingContext::traceStackRoots(JSTracer* trc) {
  TraceStackRoots(trc, stackRoots_);
}

static inline void TracePersistentRootedTraceableList(
    JSTracer* trc, mozilla::LinkedList<mc::PersistentRootedBase>& list, const char* name) {
  for (mc::PersistentRootedBase* root : list) {
    static_cast<mc::PersistentRootedTraceableBase*>(root)->trace(trc, name);
  }
}

void MCRuntime::tracePersistentRoots(JSTracer* trc) {
  JS::TracePersistentRootedLists(trc, heapRoots);

  // RootedTraceable uses virtual dispatch.
  MC::AutoSuppressGCAnalysis nogc;

  TracePersistentRootedTraceableList(
      trc, heapRoots[JS::RootKind::Traceable], "sandbox-persistent-traceable");
}

void MC::RootingContext::traceCustomAutoRooters(JSTracer* trc) {
  CustomAutoRooter* head = customAutoRooters_;
  for (CustomAutoRooter* rooter = head; rooter; rooter = rooter->down) {
    rooter->trace(trc);
  }
}

static void SandboxRootsTracer(JSTracer* trc, void* data) {
  MCContext* cx = JS_SanitizeContext((JSContext*)data);

  cx->traceCustomAutoRooters(trc);
  cx->traceStackRoots(trc);
  cx->rt_->tracePersistentRoots(trc);
}

template <typename T>
static void FinishPersistentRootedChain(
    mozilla::LinkedList<mc::PersistentRootedBase>& list) {
  while (!list.isEmpty()) {
    static_cast<MC::PersistentRooted<T>*>(list.getFirst())->reset();
  }
}

void MCRuntime::finishPersistentRoots() {
  FinishPersistentRootedChain<JS::BigInt*>(heapRoots[JS::RootKind::BigInt]);
  FinishPersistentRootedChain<JS::Symbol*>(heapRoots[JS::RootKind::Symbol]);
  FinishPersistentRootedChain<JSString*>(heapRoots[JS::RootKind::String]);
  FinishPersistentRootedChain<JSObject*>(heapRoots[JS::RootKind::Object]);
  FinishPersistentRootedChain<JSScript*>(heapRoots[JS::RootKind::Script]);
  FinishPersistentRootedChain<jsid>(heapRoots[JS::RootKind::Id]);
  FinishPersistentRootedChain<JS::Value>(heapRoots[JS::RootKind::Value]);
}

static void SandboxClearPersistentRoots(void* data) {
  MCContext* cx = JS_SanitizeContext((JSContext*)data);
  
  cx->rt_->finishPersistentRoots();
}

void MC::AddPersistentRoot(RootingContext* cx, JS::RootKind kind, mc::PersistentRootedBase* root) {
  MCRuntime* rt = static_cast<MCContext*>(cx)->rt_;
  rt->heapRoots[kind].insertBack(root);
}

void MC::AddPersistentRoot(MCRuntime* rt, JS::RootKind kind, mc::PersistentRootedBase* root) {
  rt->heapRoots[kind].insertBack(root);
}

MC::Tainted<bool> MCRuntime::enableStoreBuffer(MC::Tainted<JSRuntime*> t_rt) {
  MCRuntime* rt = t_rt.copy_and_verify_address(MC_VerifyRuntime);
  return rt->storeBuffer_.enable();
}

void MCRuntime::disableStoreBuffer(MC::Tainted<JSRuntime*> t_rt) {
  MCRuntime* rt = t_rt.copy_and_verify_address(MC_VerifyRuntime);
  rt->storeBuffer_.disable();
}

void MCRuntime::traceStoreBuffer(MC::Tainted<JSTracer*> trc, MC::Tainted<JSRuntime*> t_rt) {
  MCRuntime* rt = t_rt.copy_and_verify_address(MC_VerifyRuntime);
  rt->storeBuffer_.traceValues(trc);
  rt->storeBuffer_.traceCells(trc);
}

void MCRuntime::clearStoreBuffer(MC::Tainted<JSRuntime*> t_rt) {
  MCRuntime* rt = t_rt.copy_and_verify_address(MC_VerifyRuntime);
  rt->storeBuffer_.clear();
}

MC::Tainted<bool> MCRuntime::isEmptyStoreBuffer(MC::Tainted<JSRuntime*> t_rt) {
  MCRuntime* rt = t_rt.copy_and_verify_address(MC_VerifyRuntime);
  return rt->storeBuffer_.isEmpty();  
}

MCRuntime::MCRuntime() : storeBuffer_(this) {}

MCContext* MC_NewContext(uint32_t maxbytes, MCRuntime* parentRuntime) { 
  MOZ_RELEASE_ASSERT(!MCContext::mcx_, "Attempt to create duplication MCContext in thread");

  JSContext* jscx = JS_NewContext(maxbytes, parentRuntime == nullptr ? nullptr : parentRuntime->rt_);
  if (!jscx) {
    return nullptr;
  }

  JSRuntime* jsrt = JS_GetRuntime(jscx);
  if (!jsrt) {
    return nullptr;
  }
  
  MCContext* cx = new MCContext();
  MOZ_RELEASE_ASSERT(cx, "MCContext allocation failed!");

  MCRuntime* rt = new MCRuntime();
  MOZ_RELEASE_ASSERT(rt, "MCRuntime allocation failed!");

  rt->parent_ = parentRuntime;
  rt->rt_ = jsrt;

  cx->cx_ = jscx;
  cx->data_ = nullptr;
  cx->rt_ = rt;

  static auto SandboxRootsTracerCb = MC::Sandbox::RegisterCallback(SandboxRootsTracer);
  JS_SetSandboxStackRootsTracer(jscx, SandboxRootsTracerCb.UNSAFE_get(), jscx);

  static auto SandboxClearPersistentRootsCb = MC::Sandbox::RegisterCallback(SandboxClearPersistentRoots);
  JS_SetSandboxClearPersistentRootsCallback(jscx, SandboxClearPersistentRootsCb.UNSAFE_get(), jscx);

  static auto StoreBufferEnableCb = MC::Sandbox::RegisterTaintedCallback(MCRuntime::enableStoreBuffer);
  static auto StoreBufferDisableCb = MC::Sandbox::RegisterTaintedCallback(MCRuntime::disableStoreBuffer);
  static auto StoreBufferTraceCb = MC::Sandbox::RegisterTaintedCallback(MCRuntime::traceStoreBuffer);
  static auto StoreBufferClearCb = MC::Sandbox::RegisterTaintedCallback(MCRuntime::clearStoreBuffer);
  static auto StoreBufferIsEmptyCb = MC::Sandbox::RegisterTaintedCallback(MCRuntime::isEmptyStoreBuffer);

  static auto StoreBufferCb = JSExternalStoreBufferCallbacks{
      StoreBufferEnableCb.UNSAFE_get(),  StoreBufferDisableCb.UNSAFE_get(),
      StoreBufferTraceCb.UNSAFE_get(),   StoreBufferClearCb.UNSAFE_get(),
      StoreBufferIsEmptyCb.UNSAFE_get(),
  }; 
  
  JS_SetExternalStoreBufferCallbacks(jscx, &StoreBufferCb);

  MCContext::mcx_ = cx;
  return cx;
}

MCContext* JS_SanitizeContext(JSContext* cx) {
  MOZ_RELEASE_ASSERT(MCContext::mcx_);
  MOZ_RELEASE_ASSERT(MCContext::mcx_->cx_ == cx);
  return MCContext::mcx_;
}

MCRuntime* JS_SanitizeRuntime(JSRuntime* rt) {
  MOZ_RELEASE_ASSERT(MCContext::mcx_);
  MOZ_RELEASE_ASSERT(MCContext::mcx_->rt_->rt_ == rt);
  return MCContext::mcx_->rt_;
}
#endif
