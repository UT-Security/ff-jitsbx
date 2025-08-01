/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_RootingAPI_h
#define js_sandbox_RootingAPI_h

#include "js/RootingAPI.h"

#ifdef JS_SANDBOX

namespace MC {
namespace detail {
template <typename T>
class Rooted;
template <typename T>
class PersistentRooted;
}
}

namespace mc {

struct VirtualTraceable {
  virtual ~VirtualTraceable() = default;
  virtual void trace(JSTracer* trc, const char* name) = 0;
};

class StackRootedBase {
 public:
  StackRootedBase* previous() { return prev; }

 protected:
  StackRootedBase** stack;
  StackRootedBase* prev;

  template <typename T>
  auto* derived() {
    return static_cast<MC::detail::Rooted<T>*>(this);
  }
};

class PersistentRootedBase
    : protected mozilla::LinkedListElement<PersistentRootedBase> {
 protected:
  friend class mozilla::LinkedList<PersistentRootedBase>;
  friend class mozilla::LinkedListElement<PersistentRootedBase>;

  template <typename T>
  auto* derived() {
    return static_cast<MC::detail::PersistentRooted<T>*>(this);
  }
};

struct StackRootedTraceableBase : public StackRootedBase,
                                  public VirtualTraceable {};

class PersistentRootedTraceableBase : public PersistentRootedBase,
                                      public VirtualTraceable {};

template <typename Base, typename T>
class TypedRootedGCThingBase : public Base {
 public:
  void trace(JSTracer* trc, const char* name);
};

template <typename Base, typename T>
class TypedRootedTraceableBase : public Base {
 public:
  void trace(JSTracer* trc, const char* name) override {
    auto* self = this->template derived<T>();
    JS::GCPolicy<T>::trace(trc, self->addr(), name);
  }
};

template <typename T>
struct RootedTraceableTraits {
  using StackBase = TypedRootedTraceableBase<StackRootedTraceableBase, T>;
  using PersistentBase =
      TypedRootedTraceableBase<PersistentRootedTraceableBase, T>;
};

template <typename T>
struct RootedGCThingTraits {
  using StackBase = TypedRootedGCThingBase<StackRootedBase, T>;
  using PersistentBase = TypedRootedGCThingBase<PersistentRootedBase, T>;
};

}

namespace MC {

using RootedListHeads =
    mozilla::EnumeratedArray<JS::RootKind, JS::RootKind::Limit, mc::StackRootedBase*>;

namespace detail {

template <typename T>
constexpr bool IsTraceable_v =
    JS::MapTypeToRootKind<T>::kind == JS::RootKind::Traceable;

template <typename T>
using RootedTraits =
    std::conditional_t<IsTraceable_v<T>, mc::RootedTraceableTraits<T>,
                       mc::RootedGCThingTraits<T>>;

template <typename T>
class MOZ_RAII Rooted : public RootedTraits<T>::StackBase {
 public:
  explicit Rooted()
      : ptr(JS::SafelyInitialized<T>::create()) {}

  template <typename S>
  explicit Rooted(S&& initial) : ptr(std::forward<S>(initial)) {
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(ptr));
  }

  template <
      typename... CtorArgs>
  explicit Rooted(CtorArgs... args): ptr(std::forward<CtorArgs>(args)...) {
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(ptr));
  }

  T* addr() { return &ptr; }
  const T* addr() const { return &ptr; }

 protected:
  T ptr;
} JS_HAZ_ROOTED;

template <typename T>
class PersistentRooted : public RootedTraits<T>::PersistentBase {
 public:
  explicit PersistentRooted() : ptr(JS::SafelyInitialized<T>::create()) {}

  template <typename U>
  explicit PersistentRooted(U&& initial) : ptr(std::forward<U>(initial)) {}

  template <typename... CtorArgs>
  explicit PersistentRooted(CtorArgs... args)
      : ptr(std::forward<CtorArgs>(args)...) {}

  PersistentRooted(const PersistentRooted& rhs) : ptr(rhs.ptr) {}

  T* addr() { return &ptr; }
  const T* addr() const { return &ptr; }

 protected:
  T ptr;
} JS_HAZ_ROOTED;

} /* namespace detail */


}  // namespace MC

namespace JS {

JS_PUBLIC_API void TraceExactStackRootLists(JSTracer* trc,
                                            MC::RootedListHeads& stackRoots);

JS_PUBLIC_API void TracePersistentRootedLists(
    JSTracer* trc,
    mozilla::EnumeratedArray<JS::RootKind, JS::RootKind::Limit,
                             mozilla::LinkedList<mc::PersistentRootedBase>>&
        heapRoots);

}  // namespace JS

#endif

#endif
