/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_RootingAPI_h
#define mc_RootingAPI_h

#include "js/sandbox/RootingAPI.h"

#ifdef JS_SANDBOX

#include "monkeycage/StoreBuffer.h"
#include "monkeycage/unsafe/SandboxImpl.h"

namespace mc {

// The defaulted Enable parameter for the following two types is for restricting
// specializations with std::enable_if.
template <typename T, typename Enable = void>
struct BarrierMethods {};

}

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
class MOZ_NON_MEMMOVABLE Heap : public js::HeapOperations<T, Heap<T>> {
  // Please note: this can actually also be used by nsXBLMaybeCompiled<T>, for
  // legacy reasons.
  static_assert(js::IsHeapConstructibleType<T>::value,
                "Type T must be a public GC pointer type");

 public:
  using ElementType = T;

  Heap() : ptr(JS::SafelyInitialized<T>::create()) {
    // No barriers are required for initialization to the default value.
    static_assert(sizeof(T) == sizeof(Heap<T>),
                  "Heap<T> must be binary compatible with T.");
  }
  explicit Heap(const T& p) : ptr(p) {
    postWriteBarrier(JS::SafelyInitialized<T>::create(), ptr);
  }

  /*
   * For Heap, move semantics are equivalent to copy semantics. However, we want
   * the copy constructor to be explicit, and an explicit move constructor
   * breaks common usage of move semantics, so we need to define both, even
   * though they are equivalent.
   */
  explicit Heap(const Heap<T>& other) : ptr(other.getWithoutExpose()) {
    postWriteBarrier(JS::SafelyInitialized<T>::create(), ptr);
  }
  Heap(Heap<T>&& other) : ptr(other.getWithoutExpose()) {
    postWriteBarrier(JS::SafelyInitialized<T>::create(), ptr);
  }

  Heap& operator=(Heap<T>&& other) {
    set(other.getWithoutExpose());
    other.set(JS::SafelyInitialized<T>::create());
    return *this;
  }

  ~Heap() { postWriteBarrier(ptr, JS::SafelyInitialized<T>::create()); }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_POINTER_ASSIGN_OPS(Heap, T);

  const T* address() const { return &ptr; }

  void exposeToActiveJS() const { mc::BarrierMethods<T>::exposeToJS(ptr); }

  const T& get() const {
    exposeToActiveJS();
    return ptr;
  }
  const T& getWithoutExpose() const {
    mc::BarrierMethods<T>::readBarrier(ptr);
    return ptr;
  }
  const T& unbarrieredGet() const { return ptr; }

  void set(const T& newPtr) {
    T tmp = ptr;
    ptr = newPtr;
    postWriteBarrier(tmp, ptr);
  }

  T* unsafeGet() { return &ptr; }

  void unbarrieredSet(const T& newPtr) { ptr = newPtr; }

  explicit operator bool() const {
    return bool(mc::BarrierMethods<T>::asGCThingOrNull(ptr));
  }
  explicit operator bool() {
    return bool(mc::BarrierMethods<T>::asGCThingOrNull(ptr));
  }

 private:
  void postWriteBarrier(const T& prev, const T& next) {
    mc::BarrierMethods<T>::postWriteBarrier(&ptr, prev, next);
  }

  T ptr;
};

}  // namespace MC

namespace JS {
namespace detail {

template <typename T>
struct DefineComparisonOps<MC::Heap<T>> : std::true_type {
  static const T& get(const MC::Heap<T>& v) { return v.unbarrieredGet(); }
};

}  // namespace detail

static MOZ_ALWAYS_INLINE bool ObjectIsTenured(const MC::Heap<JSObject*>& obj) {
  return ObjectIsTenured(obj.unbarrieredGet());
}

static MOZ_ALWAYS_INLINE bool ObjectIsMarkedGray(
    const MC::Heap<JSObject*>& obj) {
  return ObjectIsMarkedGray(obj.unbarrieredGet());
}

#ifdef DEBUG
inline void AssertObjectIsNotGray(const MC::Heap<JSObject*>& obj) {
  AssertObjectIsNotGray(obj.unbarrieredGet());
}
#else
inline void AssertObjectIsNotGray(const MC::Heap<JSObject*>& obj) {}
#endif

}  // namespace JS

namespace MC {

template <typename T>
class MOZ_STACK_CLASS MutableHandle
    : public js::MutableHandleOperations<T, MutableHandle<T>> {

 public:
  using ElementType = T;

  inline MOZ_IMPLICIT MutableHandle(Rooted<T>* root);
  inline MOZ_IMPLICIT MutableHandle(PersistentRooted<T>* root);

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

 private:
  MutableHandle() = default;
  DELETE_ASSIGNMENT_OPS(MutableHandle, T);

  T* ptr;
};
}  // namespace MC

namespace mc {
namespace detail {

// Default implementations for barrier methods on secure GC thing pointers.
template <typename T>
struct PtrBarrierMethodsBase {
  static T* initial() { return nullptr; }
  static js::gc::Cell* asGCThingOrNull(T* v) {
    if (!v) {
      return nullptr;
    }
    MOZ_ASSERT(uintptr_t(v) > 32);
    return reinterpret_cast<js::gc::Cell*>(v);
  }
  static void exposeToJS(T* t) {
    if (t) {
      js::gc::ExposeGCThingToActiveJS(JS::GCCellPtr(t));
    }
  }
  static void readBarrier(T* t) {
    if (t) {
      js::gc::IncrementalReadBarrier(JS::GCCellPtr(t));
    }
  }
};

}  // namespace detail

template <typename T>
struct BarrierMethods<T*> : public detail::PtrBarrierMethodsBase<T> {
  static void postWriteBarrier(T** vp, T* prev, T* next) {
    if (next) {
      JS::AssertGCThingIsNotNurseryAllocable(
          reinterpret_cast<js::gc::Cell*>(next));
    }
  }
};

template <>
struct BarrierMethods<JSObject*>
    : public detail::PtrBarrierMethodsBase<JSObject> {
  static void postWriteBarrier(JSObject** vp, JSObject* prev, JSObject* next) {
    MC::HeapPostWriteBarrier(vp, prev, next);
  }
  static void exposeToJS(JSObject* obj) {
    if (obj) {
      JS::ExposeObjectToActiveJS(obj);
    }
  }
};


template <>
struct BarrierMethods<JSFunction*>
    : public detail::PtrBarrierMethodsBase<JSFunction> {
  static void postWriteBarrier(JSFunction** vp, JSFunction* prev,
                               JSFunction* next) {
    MC::HeapPostWriteBarrier(reinterpret_cast<JSObject**>(vp),
                             reinterpret_cast<JSObject*>(prev),
                             reinterpret_cast<JSObject*>(next));
  }
  static void exposeToJS(JSFunction* fun) {
    if (fun) {
      JS::ExposeObjectToActiveJS(reinterpret_cast<JSObject*>(fun));
    }
  }
};

template <>
struct BarrierMethods<JSString*>
    : public detail::PtrBarrierMethodsBase<JSString> {
  static void postWriteBarrier(JSString** vp, JSString* prev, JSString* next) {
    MC::HeapPostWriteBarrier(vp, prev, next);
  }
};

template <>
struct BarrierMethods<JS::BigInt*>
    : public detail::PtrBarrierMethodsBase<JS::BigInt> {
  static void postWriteBarrier(JS::BigInt** vp, JS::BigInt* prev,
                               JS::BigInt* next) {
    MC::HeapPostWriteBarrier(vp, prev, next);
  }
};

}  // namespace mc

namespace js {

template <typename T>
struct JS_PUBLIC_API StableCellHasher<JS::Heap<T>> {
  using Key = JS::Heap<T>;
  using Lookup = T;

  static bool maybeGetHash(const Lookup& l, HashNumber* hashOut) {
#ifdef JS_SANDBOX_LFI
    HashNumber* t_hashOut = (HashNumber*)monkeycage_stackpush(sizeof(HashNumber));
#else
    HashNumber* t_hashOut = hashOut;
#endif
    bool ret = StableCellHasher<T>::maybeGetHash(l, t_hashOut);
#ifdef JS_SANDBOX_LFI
    *hashOut = *t_hashOut;
    monkeycage_stackpop(sizeof(HashNumber), (void*)t_hashOut);
#endif
    return ret;
  }
  static bool ensureHash(const Lookup& l, HashNumber* hashOut) {
#ifdef JS_SANDBOX_LFI
    HashNumber* t_hashOut = (HashNumber*)monkeycage_stackpush(sizeof(HashNumber));
#else
    HashNumber* t_hashOut = hashOut;
#endif
    bool ret = StableCellHasher<T>::ensureHash(l, t_hashOut);
#ifdef JS_SANDBOX_LFI
    *hashOut = *t_hashOut;
    monkeycage_stackpop(sizeof(HashNumber), (void*)t_hashOut);
#endif
    return ret;
  }
  static HashNumber hash(const Lookup& l) {
    return StableCellHasher<T>::hash(l);
  }
  static bool match(const Key& k, const Lookup& l) {
    return StableCellHasher<T>::match(k.unbarrieredGet(), l);
  }
};

template <typename T>
struct JS_PUBLIC_API StableCellHasher<MC::Heap<T>> {
  using Key = MC::Heap<T>;
  using Lookup = T;

  static bool maybeGetHash(const Lookup& l, HashNumber* hashOut) {
#ifdef JS_SANDBOX_LFI
    HashNumber* t_hashOut = (HashNumber*)monkeycage_stackpush(sizeof(HashNumber));
#else
    HashNumber* t_hashOut = hashOut;
#endif
    bool ret = StableCellHasher<T>::maybeGetHash(l, t_hashOut);
#ifdef JS_SANDBOX_LFI
    *hashOut = *t_hashOut;
    monkeycage_stackpop(sizeof(HashNumber), (void*)t_hashOut);
#endif
    return ret;
  }
  static bool ensureHash(const Lookup& l, HashNumber* hashOut) {
#ifdef JS_SANDBOX_LFI
    HashNumber* t_hashOut = (HashNumber*)monkeycage_stackpush(sizeof(HashNumber));
#else
    HashNumber* t_hashOut = hashOut;
#endif
    bool ret = StableCellHasher<T>::ensureHash(l, t_hashOut);
#ifdef JS_SANDBOX_LFI
    *hashOut = *t_hashOut;
    monkeycage_stackpop(sizeof(HashNumber), (void*)t_hashOut);
#endif
    return ret;
  }
  static HashNumber hash(const Lookup& l) {
    return StableCellHasher<T>::hash(l);
  }
  static bool match(const Key& k, const Lookup& l) {
    return StableCellHasher<T>::match(k.unbarrieredGet(), l);
  }
};

}

struct MCContext;
struct MCRuntime;

extern MCContext* JS_SanitizeContext(JSContext*);

namespace MC {

class CustomAutoRooter;
  
using CustomAutoRooterListHead = CustomAutoRooter*;

// Superclass of MCContext which can be used for rooting data in use by the
// current thread but that does not provide all the functions of a MCContext.
class RootingContext {
  // Stack GC roots for Rooted GC heap pointers.
  RootedListHeads stackRoots_;
  template <typename T>
  friend class Rooted;

  // Stack GC roots for CustomAutoRooter classes.
  CustomAutoRooterListHead customAutoRooters_;
  friend class CustomAutoRooter;

 public:
  JSContext* cx_; 
  
  RootingContext() {
    for (auto& listHead : stackRoots_) {
      listHead = nullptr;
    }
  }

  void traceStackRoots(JSTracer* trc);
  void traceCustomAutoRooters(JSTracer* trc);

  static const RootingContext* get(const MCContext* cx) {
    return reinterpret_cast<const RootingContext*>(cx);
  }

  static RootingContext* get(MCContext* cx) {
    return reinterpret_cast<RootingContext*>(cx);
  }
};

class CustomAutoRooter {
public:
  //TODO(abhishek): remove these two overloads.
 CustomAutoRooter(JSContext* cx)
     : CustomAutoRooter(JS_SanitizeContext(cx)) {}
 CustomAutoRooter(MCContext* cx)
     : CustomAutoRooter(MC::RootingContext::get(cx)) {}
 CustomAutoRooter(RootingContext* cx)
     : down(cx->customAutoRooters_), stackTop(&cx->customAutoRooters_) {
   MOZ_ASSERT(this != *stackTop);
   *stackTop = this;
 }

protected:
  virtual ~CustomAutoRooter() {
    MOZ_ASSERT(this == *stackTop);
    *stackTop = down;
  }

  /** Supplied by derived class to trace roots */
  virtual void trace(JSTracer* trc) = 0;

private:
  friend class RootingContext;

  CustomAutoRooter* const down;
  CustomAutoRooter** const stackTop;

  /* No copy or assignment semantics */
  CustomAutoRooter(CustomAutoRooter& ida) = delete;
  void operator=(CustomAutoRooter& ida) = delete;
} JS_HAZ_ROOTED_BASE;

template <typename T>
class MOZ_RAII Rooted : public detail::Rooted<T>,
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

  //TODO(abhishek): remove this overload.
  inline RootedListHeads& rootLists(JSContext* cx) {
    return rootLists(JS_SanitizeContext(cx));
  }

 public:
  using ElementType = T;

#ifdef JS_SANDBOX_NOOP
 private:
  T stack_mem;

 public:
  template <typename RootingContext,
            typename = std::enable_if_t<std::is_copy_constructible_v<T>,
                                        RootingContext>>
  explicit Rooted(const RootingContext& cx)
      : detail::Rooted<T>(&stack_mem), stack_mem(JS::SafelyInitialized<T>::create()) {
    registerWithRootLists(rootLists(cx));
  }

  template <typename RootingContext, typename S>
  Rooted(const RootingContext& cx, S&& initial)
      : detail::Rooted<T>(&stack_mem), stack_mem(std::forward<S>(initial)) {
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(stack_mem));
    registerWithRootLists(rootLists(cx));
  }

  template <
      typename RootingContext, typename... CtorArgs,
      typename = std::enable_if_t<detail::IsTraceable_v<T>, RootingContext>>
  explicit Rooted(const RootingContext& cx, CtorArgs... args)
      : detail::Rooted<T>(&stack_mem), stack_mem(std::forward<CtorArgs>(args)...) {
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(stack_mem));
    registerWithRootLists(rootLists(cx));
  }

  ~Rooted() {
    MOZ_ASSERT(*this->stack == this);
    *this->stack = this->prev;
  }
#elif defined(JS_SANDBOX_LFI)

  template <typename RootingContext,
            typename = std::enable_if_t<std::is_copy_constructible_v<T>,
                                        RootingContext>>
  explicit Rooted(const RootingContext& cx)
      : detail::Rooted<T>(nullptr) {
    void* memory = monkeycage_stackpush(sizeof(T));
    MOZ_ASSERT(memory, "Failed to allocate sandbox stack for Rooted");
    this->ptr = (T*)memory;
    new (memory) T(JS::SafelyInitialized<T>::create());
    registerWithRootLists(rootLists(cx));
  }

  template <typename RootingContext, typename S>
  Rooted(const RootingContext& cx, S&& initial)
      : detail::Rooted<T>(nullptr) {
    void* memory = monkeycage_stackpush(sizeof(T));
    MOZ_ASSERT(memory, "Failed to allocate sandbox stack for Rooted");
    this->ptr = (T*)memory;
    new (memory) T(std::forward<S>(initial));
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(*this->ptr));
    registerWithRootLists(rootLists(cx));
  }

  template <
      typename RootingContext, typename... CtorArgs,
      typename = std::enable_if_t<detail::IsTraceable_v<T>, RootingContext>>
  explicit Rooted(const RootingContext& cx, CtorArgs... args)
      : detail::Rooted<T>(nullptr) {
    void* memory = monkeycage_stackpush(sizeof(T));
    MOZ_ASSERT(memory, "Failed to allocate sandbox stack for Rooted");
    this->ptr = (T*)memory;
    new (memory) T(std::forward<CtorArgs>(args)...);
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(*this->ptr));
    registerWithRootLists(rootLists(cx));
  }

  ~Rooted() {
    MOZ_ASSERT(*this->stack == this);
    *this->stack = this->prev;
    this->ptr->~T();
    monkeycage_stackpop(sizeof(T), (void*)this->ptr);
  }
#endif
  /*
   * This method is public for Rooted so that Codegen.py can use a Rooted
   * interchangeably with a MutableHandleValue.
   */
  void set(const T& value) {
    *this->ptr = value;
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(*this->ptr));
  }
  void set(T&& value) {
    *this->ptr = std::move(value);
    MOZ_ASSERT(JS::GCPolicy<T>::isValid(*this->ptr));
  }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_POINTER_ASSIGN_OPS(Rooted, T);

  T& get() { return *this->ptr; }
  const T& get() const { return *this->ptr; }

  T* address() { return this->ptr; }
  const T* address() const { return this->ptr; }

 private:
  Rooted(const Rooted&) = delete;
} JS_HAZ_ROOTED;

}  // namespace MC

namespace js {

inline JS::Realm* GetContextRealm(const MCContext* cx) {
  return GetContextRealm(MC::RootingContext::get(cx)->cx_);
}

inline JS::Compartment* GetContextCompartment(const MCContext* cx) {
  return GetContextCompartment(MC::RootingContext::get(cx)->cx_);
}

inline JS::Zone* GetContextZone(const MCContext* cx) {
  return GetContextZone(MC::RootingContext::get(cx)->cx_);
}

inline ProfilingStack* GetContextProfilingStackIfEnabled(MCContext* cx) {
  return GetContextProfilingStackIfEnabled(MC::RootingContext::get(cx)->cx_);
}
}  // namespace js

namespace MC {

extern void AddPersistentRoot(RootingContext* cx, JS::RootKind kind, mc::PersistentRootedBase* root);
extern void AddPersistentRoot(MCRuntime* rt, JS::RootKind kind, mc::PersistentRootedBase* root);

template <typename T>
class PersistentRooted : public detail::PersistentRooted<T>,
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
  inline void registerWithRootLists(MCContext* cx) {
    registerWithRootLists(RootingContext::get(cx));
  }

  inline void registerWithRootLists(JSContext* cx) {
    registerWithRootLists(JS_SanitizeContext(cx));
  }
 public:
  using ElementType = T;

  PersistentRooted() : detail::PersistentRooted<T>(JS::SafelyInitialized<T>::create()) {}

  template <
      typename RootHolder,
      typename = std::enable_if_t<std::is_copy_constructible_v<T>, RootHolder>>
  explicit PersistentRooted(const RootHolder& cx)
      : detail::PersistentRooted<T>(JS::SafelyInitialized<T>::create()) {
    registerWithRootLists(cx);
  }

  template <
      typename RootHolder, typename U,
      typename = std::enable_if_t<std::is_constructible_v<T, U>, RootHolder>>
  PersistentRooted(const RootHolder& cx, U&& initial)
      : detail::PersistentRooted<T>(std::forward<U>(initial)) {
    registerWithRootLists(cx);
  }

  template <typename RootHolder, typename... CtorArgs,
            typename = std::enable_if_t<detail::IsTraceable_v<T>, RootHolder>>
  explicit PersistentRooted(const RootHolder& cx, CtorArgs... args)
      : detail::PersistentRooted<T>(std::forward<CtorArgs>(args)...) {
    registerWithRootLists(cx);
  }

  PersistentRooted(const PersistentRooted& rhs) : detail::PersistentRooted<T>(rhs) {
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

  //TODO(abhishek): Remove this UNSAFE overload
  void init(JSContext* cx) { init(JS_SanitizeContext(cx)); }

  template <typename U>
  void init(RootingContext* cx, U&& initial) {
    this->ptr = std::forward<U>(initial);
    registerWithRootLists(cx);
  }
  template <typename U>
  void init(MCContext* cx, U&& initial) {
    this->ptr = std::forward<U>(initial);
    registerWithRootLists(RootingContext::get(cx));
  }
  
  //TODO(abhishek): Remove this UNSAFE overload
  template <typename U>
  void init(JSContext* cx, U&& initial) {
    init(JS_SanitizeContext(cx), initial);
  }

  void reset() {
    if (initialized()) {
      set(JS::SafelyInitialized<T>::create());
      this->remove();
    }
  }

  DECLARE_POINTER_CONSTREF_OPS(T);
  DECLARE_POINTER_ASSIGN_OPS(PersistentRooted, T);

  T& get() { return this->ptr; }
  const T& get() const { return this->ptr; }

  T* address() {
    MOZ_ASSERT(initialized());
    return &this->ptr;
  }
  const T* address() const { return &this->ptr; }

  template <typename U>
  void set(U&& value) {
    MOZ_ASSERT(initialized());
    this->ptr = std::forward<U>(value);
  }
} JS_HAZ_ROOTED;
}

namespace MC {

template <typename T>
inline MutableHandle<T>::MutableHandle(Rooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}

template <typename T>
inline MutableHandle<T>::MutableHandle(PersistentRooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}
}  // namespace MC

namespace JS {

template <typename T>
template <typename S>
inline Handle<T>::Handle(
    const MC::Rooted<S>& root,
    std::enable_if_t<std::is_convertible_v<S, T>, int> dummy) {
  ptr = reinterpret_cast<const T*>(root.address());
}

template <typename T>
template <typename S>
inline Handle<T>::Handle(
    const MC::PersistentRooted<S>& root,
    std::enable_if_t<std::is_convertible_v<S, T>, int> dummy) {
  ptr = reinterpret_cast<const T*>(root.address());
}

template <typename T>
inline MutableHandle<T>::MutableHandle(MC::Rooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}

template <typename T>
inline MutableHandle<T>::MutableHandle(MC::PersistentRooted<T>* root) {
  static_assert(sizeof(MutableHandle<T>) == sizeof(T*),
                "MutableHandle must be binary compatible with T*.");
  ptr = root->address();
}

namespace detail {

template <typename T>
struct DefineComparisonOps<MC::Rooted<T>> : std::true_type {
  static const T& get(const MC::Rooted<T>& v) { return v.get(); }
};

template <typename T>
struct DefineComparisonOps<MC::PersistentRooted<T>> : std::true_type {
  static const T& get(const MC::PersistentRooted<T>& v) { return v.get(); }
};

}

}
#else

namespace MC {

template <typename T>
using Rooted = JS::Rooted<T>;

template <typename T>
using PersistentRooted = JS::PersistentRooted<T>;

template <typename T>
using MutableHandle = JS::MutableHandle<T>;

using CustomAutoRooter = JS::CustomAutoRooter;

}

#endif
  
#endif
