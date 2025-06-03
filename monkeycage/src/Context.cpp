/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "monkeycage/Context.h"

#ifdef JS_SANDBOX 

#include "js/TracingAPI.h"
#include "mcapi.h"
#include "monkeycage/Sandbox.h"

template <typename Base, typename T>
inline void mc::TypedRootedGCThingBase<Base, T>::trace(JSTracer* trc,
                                                   const char* name) {
  auto* self = this->template derived<T>();
  JS::TraceRoot(trc, self->address(), name);
}

//TODO(abhishek): THIS IS INEFFICIENT
// We are calling into the sandbox to trace each individual root.
template <typename T>
static inline void TraceExactStackRootList(JSTracer* trc,
                                           mc::StackRootedBase* listHead,
                                           const char* name) {
  // Check size of Rooted<T> does not increase.
  static_assert(sizeof(MC::Rooted<T>) == sizeof(T) + 2 * sizeof(uintptr_t));

  for (mc::StackRootedBase* root = listHead; root; root = root->previous()) {
    static_cast<MC::Rooted<T>*>(root)->trace(trc, name);
  }
}

static inline void TraceExactStackRootTraceableList(JSTracer* trc,
                                                    mc::StackRootedBase* listHead,
                                                    const char* name) {
  for (mc::StackRootedBase* root = listHead; root; root = root->previous()) {
    static_cast<mc::StackRootedTraceableBase*>(root)->trace(trc, name);
  }
}

static inline void TraceStackRoots(JSTracer* trc,
                                   MC::RootedListHeads& stackRoots) {
  TraceExactStackRootList<JS::BigInt*>(trc, stackRoots[JS::RootKind::BigInt],
                                 "sandbox-exact-BigInt");
  TraceExactStackRootList<JS::Symbol*>(trc, stackRoots[JS::RootKind::Symbol],
                                 "sandbox-exact-Symbol");
  TraceExactStackRootList<JSString*>(trc, stackRoots[JS::RootKind::String],
                                 "sandbox-exact-String");
  TraceExactStackRootList<JSObject*>(trc, stackRoots[JS::RootKind::Object],
                                 "sandbox-exact-Object");
  TraceExactStackRootList<JSScript*>(trc, stackRoots[JS::RootKind::Script],
                                 "sandbox-exact-Script");
  TraceExactStackRootList<jsid>(trc, stackRoots[JS::RootKind::Id], "exact-id");
  TraceExactStackRootList<JS::Value>(trc, stackRoots[JS::RootKind::Value],
                                     "exact-value");

  // RootedTraceable uses virtual dispatch.
  JS::AutoSuppressGCAnalysis nogc;

  TraceExactStackRootTraceableList(trc, stackRoots[JS::RootKind::Traceable],
                                   "Traceable");
}

void MC::RootingContext::traceStackRoots(JSTracer* trc) {
  TraceStackRoots(trc, stackRoots_);
}

template <typename T>
static inline void TracePersistentRootedList(
    JSTracer* trc, mozilla::LinkedList<mc::PersistentRootedBase>& list, const char* name) {
  for (mc::PersistentRootedBase* root : list) {
    static_cast<MC::PersistentRooted<T>*>(root)->trace(trc, name);
  }
}

static inline void TracePersistentRootedTraceableList(
    JSTracer* trc, mozilla::LinkedList<mc::PersistentRootedBase>& list, const char* name) {
  for (mc::PersistentRootedBase* root : list) {
    static_cast<mc::PersistentRootedTraceableBase*>(root)->trace(trc, name);
  }
}

void MCRuntime::tracePersistentRoots(JSTracer* trc) {
  TracePersistentRootedList<JS::BigInt*>(trc, heapRoots[JS::RootKind::BigInt],
                                  "sandbox-persistent-BigInt");
  TracePersistentRootedList<JS::Symbol*>(trc, heapRoots[JS::RootKind::Symbol],
                                  "sandbox-persistent-JSSymbol");
  TracePersistentRootedList<JSString*>(trc, heapRoots[JS::RootKind::String],
                                  "sandbox-persistent-String");
  TracePersistentRootedList<JSObject*>(trc, heapRoots[JS::RootKind::Object],
                                  "sandbox-persistent-Object");
  TracePersistentRootedList<JSScript*>(trc, heapRoots[JS::RootKind::Script],
                                  "sandbox-persistent-Script");
  TracePersistentRootedList<jsid>(trc, heapRoots[JS::RootKind::Id],
                                  "sandbox-persistent-id");
  TracePersistentRootedList<JS::Value>(trc, heapRoots[JS::RootKind::Value],
                                   "sandbox-persistent-value");
  // RootedTraceable uses virtual dispatch.
  JS::AutoSuppressGCAnalysis nogc;

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
  cx->rcx_ = JS::RootingContext::get(jscx);
  cx->data_ = nullptr;
  cx->rt_ = rt;

  static auto SandboxRootsTracerCb = MC::Sandbox::RegisterCallback(SandboxRootsTracer);
  JS_SetSandboxStackRootsTracer(jscx, SandboxRootsTracerCb.UNSAFE_get(), jscx);

  static auto SandboxClearPersistentRootsCb = MC::Sandbox::RegisterCallback(SandboxClearPersistentRoots);
  JS_SetSandboxClearPersistentRootsCallback(jscx, SandboxClearPersistentRootsCb.UNSAFE_get(), jscx);
  
  MCContext::mcx_ = cx;
  return cx;
}

MCContext* JS_SanitizeContext(JSContext* cx) {
  MOZ_RELEASE_ASSERT(MCContext::mcx_);
  MOZ_RELEASE_ASSERT(MCContext::mcx_->cx_ == cx);
  return MCContext::mcx_;
}

MCContext* JS_SanitizeContext(JS::RootingContext* rcx) {
  MOZ_RELEASE_ASSERT(MCContext::mcx_);
  MOZ_RELEASE_ASSERT(MCContext::mcx_->rcx_ == rcx);
  return MCContext::mcx_;
}

#endif
