/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_RootingAPI_h
#define js_sandbox_RootingAPI_h

#include "js/RootingAPI.h"
#include "mozilla/ThreadLocal.h"

namespace JS {
namespace sandbox {

template <typename T>
class Rooted;
template <typename T>
class PersistentRooted;
}
}

namespace js {
namespace sandbox {

class StackRootedBase {
 public:
  StackRootedBase* previous() { return prev; }

 protected:
  StackRootedBase** stack;
  StackRootedBase* prev;

  template <typename T>
  auto* derived() {
    return static_cast<JS::sandbox::Rooted<T>*>(this);
  }
};  

class PersistentRootedBase
    : protected mozilla::LinkedListElement<PersistentRootedBase> {
 protected:
  friend class mozilla::LinkedList<PersistentRootedBase>;
  friend class mozilla::LinkedListElement<PersistentRootedBase>;

  template <typename T>
  auto* derived() {
    return static_cast<JS::sandbox::PersistentRooted<T>*>(this);
  }
};

struct StackRootedTraceableBase : public StackRootedBase,
                                  public VirtualTraceable {};

class PersistentRootedTraceableBase : public PersistentRootedBase,
                                      public VirtualTraceable {};

template <typename T>
struct RootedTraceableTraits {
  using StackBase = js::TypedRootedTraceableBase<StackRootedTraceableBase, T>;
  using PersistentBase =
      js::TypedRootedTraceableBase<PersistentRootedTraceableBase, T>;
};

template <typename T>
struct RootedGCThingTraits {
  using StackBase = js::TypedRootedGCThingBase<StackRootedBase, T>;
  using PersistentBase = js::TypedRootedGCThingBase<PersistentRootedBase, T>;
};

}
}

namespace JS {
namespace sandbox {

using RootedListHeads =
    mozilla::EnumeratedArray<RootKind, RootKind::Limit, js::sandbox::StackRootedBase*>;    

namespace detail {

template <typename T>
constexpr bool IsTraceable_v =
    MapTypeToRootKind<T>::kind == JS::RootKind::Traceable;

template <typename T>
using RootedTraits =
    std::conditional_t<IsTraceable_v<T>, js::sandbox::RootedTraceableTraits<T>,
                       js::sandbox::RootedGCThingTraits<T>>;

} /* namespace detail */    

typedef void (*ExternalRootingCallbackTrace)(JSTracer* trc, void* data);
typedef RootedListHeads& (*ExternalRootingCallbackRoots)(void* data);

struct JS_PUBLIC_API ExternalRootingCallbacks {
  ExternalRootingCallbackTrace trace;
  ExternalRootingCallbackRoots roots;
  ExternalRootingCallbackRoots externalRoots;
};

typedef void (*ExternalStackRootingContextTrace)(JSTracer* trc, void* data);
typedef RootedListHeads& (*ExternalStackRootingContextRoots)(void* data);

struct JS_PUBLIC_API ExternalStackRootingContext {
  void* data;

  ExternalStackRootingContextTrace trace;
  ExternalStackRootingContextRoots roots;

  ExternalStackRootingContextRoots externalRoots;
};

typedef void (*ExternalPersistentRootingCallbackTrace)(JSTracer* trc, void* data);
typedef mozilla::LinkedList<js::sandbox::PersistentRootedBase>& (*ExternalPersistentRootingCallbackRoots)(RootKind kind, void* data);

struct JS_PUBLIC_API ExternalPersistentRootingCallbacks {
  ExternalPersistentRootingCallbackTrace trace;
  ExternalPersistentRootingCallbackRoots roots;
  ExternalPersistentRootingCallbackRoots externalRoots;
};

class RootingContext : public JS::RootingContext {
public:
  ExternalRootingCallbacks externalRootingCallbacks;
  void* externalRootingCallbacksData;

  ExternalPersistentRootingCallbacks persistentRootingCallbacks;
  void* persistentRootingData;

  static const RootingContext* get(const JSContext* cx) {
    return reinterpret_cast<const RootingContext*>(cx);
  }
  
  static RootingContext* get(JSContext* cx) {
    return reinterpret_cast<RootingContext*>(cx);
  }
  
  void traceExternalStackRoots(JSTracer* trc);
};

extern JS_PUBLIC_API void JS_SetExternalRootingCallbacks(JSContext* cx, ExternalRootingCallbacks cb, void* data);

/**
 * Local variable of type T whose value is always rooted. This is typically
 * used for local variables, or for non-rooted values being passed to a
 * function that requires a handle, e.g. Foo(Root<T>(cx, x)).
 *
 * If you want to add additional methods to Rooted for a specific
 * specialization, define a RootedOperations<T> specialization containing them.
 */
template <typename T>
class MOZ_RAII Rooted : public detail::RootedTraits<T>::StackBase,
                        public js::RootedOperations<T, Rooted<T>> {
  inline void registerWithRootLists(RootedListHeads& roots) {
    this->stack = &roots[JS::MapTypeToRootKind<T>::kind];
    this->prev = *this->stack;
    *this->stack = this;
  }

  inline RootedListHeads& rootLists(RootingContext* cx) {
    return cx->externalRootingCallbacks.roots(cx->externalRootingCallbacksData);
  }

  inline RootedListHeads& rootLists(JSContext* cx) {
    return rootLists(RootingContext::get(cx));
  }

 public:
  using ElementType = T;

  // Construct an empty Rooted holding a safely initialized but empty T.
  // Requires T to have a copy constructor in order to copy the safely
  // initialized value.
  //
  // Note that for SFINAE to reject this method, the 2nd template parameter must
  // depend on RootingContext somehow even though we really only care about T.
  template <typename RootingContext,
            typename = std::enable_if_t<std::is_copy_constructible_v<T>,
                                        RootingContext>>
  explicit Rooted(const RootingContext& cx)
      : ptr(SafelyInitialized<T>::create()) {
    registerWithRootLists(rootLists(cx));
  }

  // Provide an initial value. Requires T to be constructible from the given
  // argument.
  template <typename RootingContext, typename S>
  Rooted(const RootingContext& cx, S&& initial)
      : ptr(std::forward<S>(initial)) {
    MOZ_ASSERT(GCPolicy<T>::isValid(ptr));
    registerWithRootLists(rootLists(cx));
  }

  // (Traceables only) Construct the contained value from the given arguments.
  // Constructs in-place, so T does not need to be copyable or movable.
  //
  // Note that a copyable Traceable passed only a RootingContext will
  // choose the above SafelyInitialized<T> constructor, because otherwise
  // identical functions with parameter packs are considered less specialized.
  //
  // The SFINAE type must again depend on an inferred template parameter.
  template <
      typename RootingContext, typename... CtorArgs,
      typename = std::enable_if_t<detail::IsTraceable_v<T>, RootingContext>>
  explicit Rooted(const RootingContext& cx, CtorArgs... args)
      : ptr(std::forward<CtorArgs>(args)...) {
    MOZ_ASSERT(GCPolicy<T>::isValid(ptr));
    registerWithRootLists(rootLists(cx));
  }

  ~Rooted() {
    MOZ_ASSERT(*this->stack == this);
    *this->stack = this->prev;
  }

  /*
   * This method is public for Rooted so that Codegen.py can use a Rooted
   * interchangeably with a MutableHandleValue.
   */
  void set(const T& value) {
    ptr = value;
    MOZ_ASSERT(GCPolicy<T>::isValid(ptr));
  }
  void set(T&& value) {
    ptr = std::move(value);
    MOZ_ASSERT(GCPolicy<T>::isValid(ptr));
  }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_POINTER_ASSIGN_OPS(Rooted, T);

  T& get() { return ptr; }
  const T& get() const { return ptr; }

  T* address() { return &ptr; }
  const T* address() const { return &ptr; }

 private:
  T ptr;

  Rooted(const Rooted&) = delete;
} JS_HAZ_ROOTED;


struct RuntimeRootingContext {
  ExternalPersistentRootingCallbacks persistentRootingCallbacks;
  void* persistentRootingData;

  static const RuntimeRootingContext* get(const JSRuntime* rt) {
    return reinterpret_cast<const RuntimeRootingContext*>(rt);
  }

  static RuntimeRootingContext* get(JSRuntime* rt) {
    return reinterpret_cast<RuntimeRootingContext*>(rt);
  }
};

extern JS_PUBLIC_API void JS_SetPersistentRootingCallbacks(JSContext* cx, ExternalPersistentRootingCallbacks cb, void* data);

template <typename T>
class PersistentRooted : public detail::RootedTraits<T>::PersistentBase,
                         public js::RootedOperations<T, PersistentRooted<T>> {

  void registerWithRootLists(JS::sandbox::RuntimeRootingContext* rt) {
    MOZ_ASSERT(!initialized());
    JS::RootKind kind = JS::MapTypeToRootKind<T>::kind;
    rt->persistentRootingCallbacks.roots(kind, rt->persistentRootingData).insertBack(this);
  }
  
  void registerWithRootLists(JSRuntime* rt) {
    registerWithRootLists(JS::sandbox::RuntimeRootingContext::get(rt));
  }
  
  void registerWithRootLists(RootingContext* cx) {
    MOZ_ASSERT(!initialized());
    JS::RootKind kind = JS::MapTypeToRootKind<T>::kind;
    cx->persistentRootingCallbacks.roots(kind, cx->persistentRootingData).insertBack(this);
  }

  // Used when JSContext type is incomplete and so it is not known to inherit
  // from RootingContext.
  void registerWithRootLists(JSContext* cx) {
    registerWithRootLists(RootingContext::get(cx));
  }

 public:
  using ElementType = T;

  PersistentRooted() : ptr(SafelyInitialized<T>::create()) {}

  template <
      typename RootHolder,
      typename = std::enable_if_t<std::is_copy_constructible_v<T>, RootHolder>>
  explicit PersistentRooted(const RootHolder& cx)
      : ptr(SafelyInitialized<T>::create()) {
    registerWithRootLists(cx);
  }

  template <
      typename RootHolder, typename U,
      typename = std::enable_if_t<std::is_constructible_v<T, U>, RootHolder>>
  PersistentRooted(const RootHolder& cx, U&& initial)
      : ptr(std::forward<U>(initial)) {
    registerWithRootLists(cx);
  }

  template <typename RootHolder, typename... CtorArgs,
            typename = std::enable_if_t<detail::IsTraceable_v<T>, RootHolder>>
  explicit PersistentRooted(const RootHolder& cx, CtorArgs... args)
      : ptr(std::forward<CtorArgs>(args)...) {
    registerWithRootLists(cx);
  }

  PersistentRooted(const PersistentRooted& rhs) : ptr(rhs.ptr) {
    /*
     * Copy construction takes advantage of the fact that the original
     * is already inserted, and simply adds itself to whatever list the
     * original was on - no JSRuntime pointer needed.
     *
     * This requires mutating rhs's links, but those should be 'mutable'
     * anyway. C++ doesn't let us declare mutable base classes.
     */
    const_cast<PersistentRooted&>(rhs).setNext(this);
  }

  bool initialized() const { return this->isInList(); }

  void init(RootingContext* cx) { init(cx, SafelyInitialized<T>::create()); }
  void init(JSContext* cx) { init(RootingContext::get(cx)); }

  template <typename U>
  void init(RootingContext* cx, U&& initial) {
    ptr = std::forward<U>(initial);
    registerWithRootLists(cx);
  }
  template <typename U>
  void init(JSContext* cx, U&& initial) {
    ptr = std::forward<U>(initial);
    registerWithRootLists(RootingContext::get(cx));
  }

  void reset() {
    if (initialized()) {
      set(SafelyInitialized<T>::create());
      this->remove();
    }
  }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_POINTER_ASSIGN_OPS(PersistentRooted, T);

  T& get() { return ptr; }
  const T& get() const { return ptr; }

  T* address() {
    MOZ_ASSERT(initialized());
    return &ptr;
  }
  const T* address() const { return &ptr; }

  template <typename U>
  void set(U&& value) {
    MOZ_ASSERT(initialized());
    ptr = std::forward<U>(value);
  }

 private:
  T ptr;
} JS_HAZ_ROOTED;

}

namespace detail {
template <typename T>
struct DefineComparisonOps<::JS::sandbox::Rooted<T>> : std::true_type {
  static const T& get(const ::JS::sandbox::Rooted<T>& v) { return v.get(); }
};
}  // namespace detail

}


namespace JS {

template <typename T>
template <typename S>
inline Handle<T>::Handle(
    const sandbox::Rooted<S>& root,
    std::enable_if_t<std::is_convertible_v<S, T>, int> dummy) {
  ptr = reinterpret_cast<const T*>(root.address());
}

template <typename T>
template <typename S>
inline Handle<T>::Handle(
    const sandbox::PersistentRooted<S>& root,
    std::enable_if_t<std::is_convertible_v<S, T>, int> dummy) {
  ptr = reinterpret_cast<const T*>(root.address());
}

template <typename T>
inline MutableHandle<T>::MutableHandle(sandbox::Rooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}

template <typename T>
inline MutableHandle<T>::MutableHandle(sandbox::PersistentRooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}
}
#endif
