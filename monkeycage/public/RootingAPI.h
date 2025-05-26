/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_RootingAPI_h
#define mc_RootingAPI_h

#include "js/RootingAPI.h"

#ifdef JS_SANDBOX

namespace MC {

template <typename T>
class MutableHandle;
template <typename T>
class Rooted;
template <typename T>
class PersistentRooted;

}

namespace MC {

template <typename T>
class MOZ_STACK_CLASS MutableHandle
    : public js::MutableHandleOperations<T, MutableHandle<T>> {

 public:
  using ElementType = T;

  inline MOZ_IMPLICIT MutableHandle(Rooted<T>* root);
  //inline MOZ_IMPLICIT MutableHandle(PersistentRooted<T>* root);

 private:
  // Disallow nullptr for overloading purposes.
  MutableHandle(decltype(nullptr)) = delete;

 public:
  MutableHandle(const MutableHandle<T>&) = default;
  void set(const T& v) {
    *ptr = v;
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(*ptr));
  }
  void set(T&& v) {
    *ptr = std::move(v);
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(*ptr));
  }

  static MutableHandle fromMarkedLocation(T* p) {
    MutableHandle h;
    h.ptr = p;
    return h;
  }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_NONPOINTER_ACCESSOR_METHODS(*ptr);
  DECLARE_NONPOINTER_MUTABLE_ACCESSOR_METHODS(*ptr);
  
  //INTERNAL CONVERSION OPERATIONS
  inline JS::MutableHandle<T> MC_INTERNAL_SAFE_get() {
    return JS::MutableHandle<T>::fromMarkedLocation(address());
  }

  //TODO(abhishek): UNSAFE CONVERSION
  inline MOZ_IMPLICIT operator JS::MutableHandle<T>() {
    return JS::MutableHandle<T>::fromMarkedLocation(address());
  }

 private:
  MutableHandle() = default;
  DELETE_ASSIGNMENT_OPS(MutableHandle, T);

  T* ptr;
};
}  // namespace MC

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
    return static_cast<MC::Rooted<T>*>(this);
  }
};

class PersistentRootedBase
    : protected mozilla::LinkedListElement<PersistentRootedBase> {
 protected:
  friend class mozilla::LinkedList<PersistentRootedBase>;
  friend class mozilla::LinkedListElement<PersistentRootedBase>;

  template <typename T>
  auto* derived() {
    return static_cast<MC::PersistentRooted<T>*>(this);
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
    JS::GCPolicy<T>::trace(trc, self->address(), name);
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
}  // namespace mc

struct MCContext;
struct MCRuntime;

namespace MC {

using RootedListHeads =
    mozilla::EnumeratedArray<JS::RootKind, JS::RootKind::Limit, mc::StackRootedBase*>;
    
// Superclass of MCContext which can be used for rooting data in use by the
// current thread but that does not provide all the functions of a MCContext.
class RootingContext {
  // Stack GC roots for Rooted GC heap pointers.
  RootedListHeads stackRoots_;
  template <typename T>
  friend class Rooted;

 public:
  RootingContext() {
    for (auto& listHead : stackRoots_) {
      listHead = nullptr;
    }
  }

  void traceStackRoots(JSTracer* trc);

  static const RootingContext* get(const MCContext* cx) {
    return reinterpret_cast<const RootingContext*>(cx);
  }

  static RootingContext* get(MCContext* cx) {
    return reinterpret_cast<RootingContext*>(cx);
  }
};

namespace detail {

template <typename T>
constexpr bool IsTraceable_v =
    JS::MapTypeToRootKind<T>::kind == JS::RootKind::Traceable;

template <typename T>
using RootedTraits =
    std::conditional_t<IsTraceable_v<T>, mc::RootedTraceableTraits<T>,
                       mc::RootedGCThingTraits<T>>;

} /* namespace detail */

template <typename T>
class MOZ_RAII Rooted : public detail::RootedTraits<T>::StackBase,
                        public js::RootedOperations<T, Rooted<T>> {
  inline void registerWithRootLists(RootedListHeads& roots) {
    this->stack = &roots[JS::MapTypeToRootKind<T>::kind];
    this->prev = *this->stack;
    *this->stack = this;
  }

  inline RootedListHeads& rootLists(RootingContext* cx) {
    return cx->stackRoots_;
  }

  inline RootedListHeads& rootLists(MCContext* cx) {
    return rootLists(RootingContext::get(cx));
  }

 public:
  using ElementType = T;

  template <typename RootingContext,
            typename = std::enable_if_t<std::is_copy_constructible_v<T>,
                                        RootingContext>>
  explicit Rooted(const RootingContext& cx)
      : ptr(JS::SafelyInitialized<T>::create()) {
    registerWithRootLists(rootLists(cx));
  }

  template <typename RootingContext, typename S>
  Rooted(const RootingContext& cx, S&& initial)
      : ptr(std::forward<S>(initial)) {
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(ptr));
    registerWithRootLists(rootLists(cx));
  }

  template <
      typename RootingContext, typename... CtorArgs,
      typename = std::enable_if_t<detail::IsTraceable_v<T>, RootingContext>>
  explicit Rooted(const RootingContext& cx, CtorArgs... args)
      : ptr(std::forward<CtorArgs>(args)...) {
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(ptr));
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
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(ptr));
  }
  void set(T&& value) {
    ptr = std::move(value);
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(ptr));
  }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_POINTER_ASSIGN_OPS(Rooted, T);

  T& get() { return ptr; }
  const T& get() const { return ptr; }

  T* address() { return &ptr; }
  const T* address() const { return &ptr; }

  //TODO(abhishek): UNSAFE CONVERSION OPERATIONS
  // These should be removed once we have safe Tainted MCAPI
  // versions of Handle and MutableHandle.

  template <typename S = T,
            typename = std::enable_if_t<std::is_convertible_v<S, T>, T>>
  inline MOZ_IMPLICIT operator JS::Handle<S>() const {
    return JS::Handle<S>::fromMarkedLocation(address());
  }

  template <typename S = T,
            typename = std::enable_if_t<std::is_convertible_v<S, T>, T>>
  inline MOZ_IMPLICIT operator JS::MutableHandle<S>() {
    return JS::MutableHandle<S>::fromMarkedLocation(address());
  }

 private:
  T ptr;

  Rooted(const Rooted&) = delete;
} JS_HAZ_ROOTED;

}  // namespace MC


namespace MC {

template <typename T>
inline MutableHandle<T>::MutableHandle(Rooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}

}

namespace MC {

extern void AddPersistentRoot(RootingContext* cx, JS::RootKind kind, mc::PersistentRootedBase* root);
extern void AddPersistentRoot(MCRuntime* rt, JS::RootKind kind, mc::PersistentRootedBase* root);

template <typename T>
class PersistentRooted : public detail::RootedTraits<T>::PersistentBase,
                         public js::RootedOperations<T, PersistentRooted<T>> {
  void registerWithRootLists(RootingContext* cx) {
    MOZ_ASSERT(!initialized());
    JS::RootKind kind = JS::MapTypeToRootKind<T>::kind;
    AddPersistentRoot(cx, kind, this);
  }

  void registerWithRootLists(MCRuntime* rt) {
    MOZ_ASSERT(!initialized());
    JS::RootKind kind = JS::MapTypeToRootKind<T>::kind;
    AddPersistentRoot(rt, kind, this);
  }

  // Used when JSContext type is incomplete and so it is not known to inherit
  // from RootingContext.
  void registerWithRootLists(MCContext* cx) {
    registerWithRootLists(RootingContext::get(cx));
  }

 public:
  using ElementType = T;

  PersistentRooted() : ptr(JS::SafelyInitialized<T>::create()) {}

  template <
      typename RootHolder,
      typename = std::enable_if_t<std::is_copy_constructible_v<T>, RootHolder>>
  explicit PersistentRooted(const RootHolder& cx)
      : ptr(JS::SafelyInitialized<T>::create()) {
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

  void init(RootingContext* cx) { init(cx, JS::SafelyInitialized<T>::create()); }
  void init(MCContext* cx) { init(RootingContext::get(cx)); }

  template <typename U>
  void init(RootingContext* cx, U&& initial) {
    ptr = std::forward<U>(initial);
    registerWithRootLists(cx);
  }
  template <typename U>
  void init(MCContext* cx, U&& initial) {
    ptr = std::forward<U>(initial);
    registerWithRootLists(RootingContext::get(cx));
  }

  void reset() {
    if (initialized()) {
      set(JS::SafelyInitialized<T>::create());
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
#else

namespace MC {

template <typename T>
using Rooted = JS::Rooted<T>;

template <typename T>
using PersistentRooted = JS::PersistentRooted<T>;

template <typename T>
using MutableHandle = JS::MutableHandle<T>;

}

#endif
  
#endif
