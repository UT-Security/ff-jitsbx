/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/TracingAPI.h"
#include "gc/Tracer.h"
#include "js/sandbox/RootingAPI.h"

template <typename Base, typename T>
inline void mc::TypedRootedGCThingBase<Base, T>::trace(JSTracer* trc,
                                                   const char* name) {
  auto* self = this->template derived<T>();
  JS::TraceRoot(trc, self->addr(), name);
}


template <typename T>
static inline void TraceExactStackRootList(JSTracer* trc,
                                           mc::StackRootedBase* listHead,
                                           const char* name) {
  // Check size of Rooted<T> does not increase.
  static_assert(sizeof(MC::detail::Rooted<T>) == sizeof(T) + 2 * sizeof(uintptr_t));

  for (mc::StackRootedBase* root = listHead; root; root = root->previous()) {
    static_cast<MC::detail::Rooted<T>*>(root)->mc::template TypedRootedGCThingBase<mc::StackRootedBase, T>::trace(trc, name);
  }
}

JS_PUBLIC_API void JS::TraceExactStackRootLists(JSTracer* trc, MC::RootedListHeads& stackRoots) {
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
}

template <typename T>
static inline void TracePersistentRootedList(
    JSTracer* trc, mozilla::LinkedList<mc::PersistentRootedBase>& list, const char* name) {
  for (mc::PersistentRootedBase* root : list) {
    static_cast<MC::detail::PersistentRooted<T>*>(root)->mc::template TypedRootedGCThingBase<mc::PersistentRootedBase, T>::trace(trc, name);
  }
}

JS_PUBLIC_API void JS::TracePersistentRootedLists(
    JSTracer* trc,
    mozilla::EnumeratedArray<JS::RootKind, JS::RootKind::Limit,
                             mozilla::LinkedList<mc::PersistentRootedBase>>& heapRoots) {
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
}
