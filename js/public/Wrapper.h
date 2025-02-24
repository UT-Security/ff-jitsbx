/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_Wrapper_h
#define js_Wrapper_h

#include "mozilla/Attributes.h"

#include "js/Proxy.h"

namespace js {
struct CompartmentFilter;

/*
 * Helper for Wrapper::New default options.
 *
 * Callers of Wrapper::New() who wish to specify a prototype for the created
 * Wrapper, *MUST* construct a WrapperOptions with a JSContext.
 */
class MOZ_STACK_CLASS WrapperOptions : public ProxyOptions {
 public:
  WrapperOptions() : ProxyOptions(false), proto_() {}

  explicit WrapperOptions(JSContext* cx) : ProxyOptions(false), proto_() {
    proto_.emplace(cx);
  }

  inline JSObject* proto() const;
  WrapperOptions& setProto(JSObject* protoArg) {
    MOZ_ASSERT(proto_);
    *proto_ = protoArg;
    return *this;
  }

 private:
  mozilla::Maybe<JS::RootedObject> proto_;
};

// Base class for proxy handlers that want to forward all operations to an
// object stored in the proxy's private slot.
class JS_PUBLIC_API ForwardingProxyHandler : public BaseProxyHandler {
 public:
  using BaseProxyHandler::BaseProxyHandler;

  /* Standard internal methods. */
  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override;
  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override;
  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const override;
  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const override;
  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override;
  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const override;
  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override;
  virtual bool getPrototypeIfOrdinary(
      JSContext* cx, JS::HandleObject proxy, bool* isOrdinary,
      JS::MutableHandleObject protop) const override;
  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override;
  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override;
  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override;
  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const override;
  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override;
  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override;
  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const override;
  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const override;

  /* SpiderMonkey extensions. */
  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const override;
  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const override;
  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override;
  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               ESClass* cls) const override;
  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override;
  virtual const char* className(JSContext* cx,
                                JS::HandleObject proxy) const override;
  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const override;
  virtual RegExpShared* regexp_toShared(JSContext* cx,
                                        JS::HandleObject proxy) const override;
  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override;
  virtual bool isCallable(JSObject* obj) const override;
  virtual bool isConstructor(JSObject* obj) const override;

  // Use the target object for private fields.
  virtual bool useProxyExpandoObjectForPrivateFields() const override {
    return false;
  }
};

#ifdef JS_SANDBOX
DECLARE_PROXY_HANDLER_OPS_CLASS(ForwardingProxyHandlerWithOps, ForwardingProxyHandler)
#endif

/*
 * A wrapper is a proxy with a target object to which it generally forwards
 * operations, but may restrict access to certain operations or augment those
 * operations in various ways.
 *
 * A wrapper can be "unwrapped" in C++, exposing the underlying object.
 * Callers should be careful to avoid unwrapping security wrappers in the wrong
 * context.
 *
 * Important: If you add a method implementation here, you probably also need
 * to add an override in CrossCompartmentWrapper. If you don't, you risk
 * compartment mismatches. See bug 945826 comment 0.
 */
class JS_PUBLIC_API Wrapper : public ForwardingProxyHandler {
  unsigned mFlags;

 public:
#ifdef JS_SANDBOX
  explicit Wrapper(unsigned aFlags, bool aHasPrototype = false,
                   bool aHasSecurityPolicy = false);
#else
  explicit constexpr Wrapper(unsigned aFlags, bool aHasPrototype = false,
                   bool aHasSecurityPolicy = false)
      : ForwardingProxyHandler(&family, aHasPrototype,
                               aHasSecurityPolicy),
        mFlags(aFlags) {}
#endif

  virtual bool finalizeInBackground(const JS::Value& priv) const override;

  /**
   * A hook subclasses can override to implement CheckedUnwrapDynamic
   * behavior.  The JSContext represents the "who is trying to unwrap?" Realm.
   * The JSObject is the wrapper that the caller is trying to unwrap.
   */
  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,
                                           JSContext* cx) const {
    MOZ_ASSERT(hasSecurityPolicy(), "Why are you asking?");
    return false;
  }

  using BaseProxyHandler::Action;

  enum Flags { CROSS_COMPARTMENT = 1 << 0, LAST_USED_FLAG = CROSS_COMPARTMENT };

  static JSObject* New(JSContext* cx, JSObject* obj, const Wrapper* handler,
                       const WrapperOptions& options = WrapperOptions());

  static JSObject* Renew(JSObject* existing, JSObject* obj,
                         const Wrapper* handler);

  static inline const Wrapper* wrapperHandler(const JSObject* wrapper);

  static JSObject* wrappedObject(JSObject* wrapper);

  unsigned flags() const { return mFlags; }

  bool isCrossCompartmentWrapper() const {
    return !!(mFlags & CROSS_COMPARTMENT);
  }

  JS_PUBLIC_API static const char* family_p();

  static const char family;
  static const Wrapper singleton;
  static const Wrapper singletonWithPrototype;

  static JSObject* const defaultProto;
};

#ifdef JS_SANDBOX
typedef bool (*WrapperDynamicCheckedUnwrapAllowedOp)(const void* p, JS::HandleObject obj, JSContext* cx);

struct JS_PUBLIC_API WrapperOps: public ProxyHandlerOps {
  WrapperDynamicCheckedUnwrapAllowedOp dynamicCheckedUnwrapAllowed;
};

#define DECLARE_WRAPPER_OPS_METHODS                                        \
  DECLARE_PROXY_HANDLER_OPS_METHODS                                        \
  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,           \
                                           JSContext* cx) const override;

#define DEFINE_WRAPPER_OPS_METHODS(name, ops, handler)                     \
DEFINE_PROXY_HANDLER_OPS_METHODS(name, ops, handler)                       \
bool name::dynamicCheckedUnwrapAllowed(JS::HandleObject obj,               \
                                           JSContext* cx) const {          \
  return ops->dynamicCheckedUnwrapAllowed(handler, obj, cx);               \
}

#define DECLARE_WRAPPER_OPS_CLASS(name, base)                                \
class JS_PUBLIC_API name : public base {                                     \
  const WrapperOps* pOps_;                                                   \
  const void* private_;                                                      \
public:                                                                      \
  explicit name(const WrapperOps* pOps, const void* priv,                    \
                            unsigned aFlags, bool aHasPrototype = false,     \
                            bool aHasSecurityPolicy = false);                \
  const WrapperOps* getOps() const { return pOps_; }                         \
  const void* getPrivate() const { return private_; }                        \
  DECLARE_WRAPPER_OPS_METHODS                                                \
};

#define DEFINE_WRAPPER_OPS_CLASS(name, base)                                  \
name::name(const WrapperOps* pOps,                                            \
                                     const void* priv,                        \
                                     unsigned aFlags,                         \
                                     bool aHasPrototype,                      \
                                     bool aHasSecurityPolicy)                 \
     : base(aFlags, aHasPrototype, aHasSecurityPolicy),                       \
       pOps_(pOps), private_(priv) {}                                         \
DEFINE_WRAPPER_OPS_METHODS(name, pOps_, private_)

DECLARE_WRAPPER_OPS_CLASS(WrapperWithOps, Wrapper)
#endif

inline JSObject* WrapperOptions::proto() const {
  return proto_ ? *proto_ : Wrapper::defaultProto;
}

/* Base class for all cross compartment wrapper handlers. */
class JS_PUBLIC_API CrossCompartmentWrapper : public Wrapper {
 public:
  explicit inline CrossCompartmentWrapper(unsigned aFlags,
                                             bool aHasPrototype = false,
                                             bool aHasSecurityPolicy = false)
      : Wrapper(CROSS_COMPARTMENT | aFlags, aHasPrototype, aHasSecurityPolicy) {
  }

  /* Standard internal methods. */
  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override;
  virtual bool defineProperty(JSContext* cx, JS::HandleObject wrapper,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override;
  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject wrapper,
                               JS::MutableHandleIdVector props) const override;
  virtual bool delete_(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                       JS::ObjectOpResult& result) const override;
  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override;
  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const override;
  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override;

  virtual bool getPrototypeIfOrdinary(
      JSContext* cx, JS::HandleObject proxy, bool* isOrdinary,
      JS::MutableHandleObject protop) const override;
  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override;
  virtual bool preventExtensions(JSContext* cx, JS::HandleObject wrapper,
                                 JS::ObjectOpResult& result) const override;
  virtual bool isExtensible(JSContext* cx, JS::HandleObject wrapper,
                            bool* extensible) const override;
  virtual bool has(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                   bool* bp) const override;
  virtual bool get(JSContext* cx, JS::HandleObject wrapper,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override;
  virtual bool set(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override;
  virtual bool call(JSContext* cx, JS::HandleObject wrapper,
                    const JS::CallArgs& args) const override;
  virtual bool construct(JSContext* cx, JS::HandleObject wrapper,
                         const JS::CallArgs& args) const override;

  /* SpiderMonkey extensions. */
  virtual bool hasOwn(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                      bool* bp) const override;
  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject wrapper,
      JS::MutableHandleIdVector props) const override;
  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override;
  virtual const char* className(JSContext* cx,
                                JS::HandleObject proxy) const override;
  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject wrapper,
                                 bool isToSource) const override;
  virtual RegExpShared* regexp_toShared(JSContext* cx,
                                        JS::HandleObject proxy) const override;
  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override;

  // Allocate CrossCompartmentWrappers in the nursery.
  virtual bool canNurseryAllocate() const override { return true; }

  static const CrossCompartmentWrapper singleton;
  static const CrossCompartmentWrapper singletonWithPrototype;

  static const CrossCompartmentWrapper getSingleton();
  static const CrossCompartmentWrapper* getSingletonP();
};

#ifdef JS_SANDBOX
DECLARE_WRAPPER_OPS_CLASS(CrossCompartmentWrapperWithOps, CrossCompartmentWrapper)
#endif

class JS_PUBLIC_API OpaqueCrossCompartmentWrapper
    : public CrossCompartmentWrapper {
 public:
  explicit inline OpaqueCrossCompartmentWrapper()
      : CrossCompartmentWrapper(0) {}

  /* Standard internal methods. */
  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override;
  virtual bool defineProperty(JSContext* cx, JS::HandleObject wrapper,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override;
  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject wrapper,
                               JS::MutableHandleIdVector props) const override;
  virtual bool delete_(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                       JS::ObjectOpResult& result) const override;
  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override;
  virtual bool getPrototype(JSContext* cx, JS::HandleObject wrapper,
                            JS::MutableHandleObject protop) const override;
  virtual bool setPrototype(JSContext* cx, JS::HandleObject wrapper,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override;
  virtual bool getPrototypeIfOrdinary(
      JSContext* cx, JS::HandleObject wrapper, bool* isOrdinary,
      JS::MutableHandleObject protop) const override;
  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject wrapper,
                                     bool* succeeded) const override;
  virtual bool preventExtensions(JSContext* cx, JS::HandleObject wrapper,
                                 JS::ObjectOpResult& result) const override;
  virtual bool isExtensible(JSContext* cx, JS::HandleObject wrapper,
                            bool* extensible) const override;
  virtual bool has(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                   bool* bp) const override;
  virtual bool get(JSContext* cx, JS::HandleObject wrapper,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override;
  virtual bool set(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override;
  virtual bool call(JSContext* cx, JS::HandleObject wrapper,
                    const JS::CallArgs& args) const override;
  virtual bool construct(JSContext* cx, JS::HandleObject wrapper,
                         const JS::CallArgs& args) const override;

  /* SpiderMonkey extensions. */
  virtual bool hasOwn(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                      bool* bp) const override;
  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject wrapper,
      JS::MutableHandleIdVector props) const override;
  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject wrapper,
                               ESClass* cls) const override;
  virtual bool isArray(JSContext* cx, JS::HandleObject obj,
                       JS::IsArrayAnswer* answer) const override;
  virtual const char* className(JSContext* cx,
                                JS::HandleObject wrapper) const override;
  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const override;

  static const OpaqueCrossCompartmentWrapper singleton;
  static const OpaqueCrossCompartmentWrapper* getSingletonP();
};

#ifdef JS_SANDBOX

#define DECLARE_OPAQUE_WRAPPER_OPS_CLASS(name, base)                         \
class JS_PUBLIC_API name : public base {                                     \
  const WrapperOps* pOps_;                                                   \
  const void* private_;                                                      \
public:                                                                      \
  explicit name(const WrapperOps* pOps, const void* priv);                   \
  const WrapperOps* getOps() const { return pOps_; }                         \
  const void* getPrivate() const { return private_; }                        \
  DECLARE_WRAPPER_OPS_METHODS                                                \
};

#define DEFINE_OPAQUE_WRAPPER_OPS_CLASS(name, base)                           \
name::name(const WrapperOps* pOps, const void* priv)                          \
     : base(), pOps_(pOps), private_(priv) {}                                 \
DEFINE_WRAPPER_OPS_METHODS(name, pOps_, private_)

DECLARE_OPAQUE_WRAPPER_OPS_CLASS(OpaqueCrossCompartmentWrapperWithOps, OpaqueCrossCompartmentWrapper)
#endif

/*
 * Base class for security wrappers. A security wrapper is potentially hiding
 * all or part of some wrapped object thus SecurityWrapper defaults to denying
 * access to the wrappee. This is the opposite of Wrapper which tries to be
 * completely transparent.
 *
 * NB: Currently, only a few ProxyHandler operations are overridden to deny
 * access, relying on derived SecurityWrapper to block access when necessary.
 */
template <class Base>
class JS_PUBLIC_API SecurityWrapper : public Base {
 public:
  explicit constexpr SecurityWrapper(unsigned flags, bool hasPrototype = false)
      : Base(flags, hasPrototype, /* hasSecurityPolicy = */ true) {}

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     Wrapper::Action act, bool mayThrow,
                     bool* bp) const override;

  virtual bool defineProperty(JSContext* cx, JS::HandleObject wrapper,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override;
  virtual bool isExtensible(JSContext* cx, JS::HandleObject wrapper,
                            bool* extensible) const override;
  virtual bool preventExtensions(JSContext* cx, JS::HandleObject wrapper,
                                 JS::ObjectOpResult& result) const override;
  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override;
  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override;

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override;
  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject wrapper,
                               ESClass* cls) const override;
  virtual bool isArray(JSContext* cx, JS::HandleObject wrapper,
                       JS::IsArrayAnswer* answer) const override;
  virtual RegExpShared* regexp_toShared(JSContext* cx,
                                        JS::HandleObject proxy) const override;
  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override;

  // Allow isCallable and isConstructor. They used to be class-level, and so
  // could not be guarded against.

  /*
   * Allow our subclasses to select the superclass behavior they want without
   * needing to specify an exact superclass.
   */
  typedef Base Permissive;
  typedef SecurityWrapper<Base> Restrictive;
};

#ifdef JS_SANDBOX

#define DECLARE_SECURITY_WRAPPER_OPS_METHODS                                     \
  bool finalizeInBackground(const JS::Value& priv) const override;               \
  bool canNurseryAllocate() const override;                                      \
  bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,           \
             Wrapper::Action act, bool mayThrow, bool* bp) const override;       \
  bool getOwnPropertyDescriptor(                                                 \
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                    \
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)            \
      const override;                                                            \
  bool defineProperty(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,    \
                      JS::Handle<JS::PropertyDescriptor> desc,                   \
                      JS::ObjectOpResult& result) const override;                \
  bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,                    \
                       JS::MutableHandleIdVector props) const override;          \
  bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,           \
               JS::ObjectOpResult& result) const override;                       \
  bool getPrototype(JSContext* cx, JS::HandleObject proxy,                       \
                    JS::MutableHandleObject protop) const override;              \
  bool setPrototype(JSContext* cx, JS::HandleObject proxy,                       \
                    JS::HandleObject proto,                                      \
                    JS::ObjectOpResult& result) const override;                  \
  bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,             \
                              bool* isOrdinary,                                  \
                              JS::MutableHandleObject protop) const override;    \
  bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,              \
                             bool* succeeded) const override;                    \
  bool preventExtensions(JSContext* cx, JS::HandleObject proxy,                  \
                         JS::ObjectOpResult& result) const override;             \
  bool isExtensible(JSContext* cx, JS::HandleObject proxy,                       \
                    bool* extensible) const override;                            \
  bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,               \
           bool* bp) const override;                                             \
  bool get(JSContext* cx, JS::HandleObject proxy, JS::HandleValue receiver,      \
           JS::HandleId id, JS::MutableHandleValue vp) const override;           \
  bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,               \
           JS::HandleValue v, JS::HandleValue receiver,                          \
           JS::ObjectOpResult& result) const override;                           \
  bool useProxyExpandoObjectForPrivateFields() const override;                   \
  bool throwOnPrivateField() const override;                                     \
  bool call(JSContext* cx, JS::HandleObject proxy,                               \
            const JS::CallArgs& args) const override;                            \
  bool construct(JSContext* cx, JS::HandleObject proxy,                          \
                 const JS::CallArgs& args) const override;                       \
  bool enumerate(JSContext* cx, JS::HandleObject proxy,                          \
                 JS::MutableHandleIdVector props) const override;                \
  bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,            \
              bool* bp) const override;                                          \
  bool getOwnEnumerablePropertyKeys(                                             \
      JSContext* cx, JS::HandleObject proxy,                                     \
      JS::MutableHandleIdVector props) const override;                           \
  bool nativeCall(JSContext* cx, JS::IsAcceptableThis test, JS::NativeImpl impl, \
                  const JS::CallArgs& args) const override;                      \
  bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,                    \
                       ESClass* cls) const override;                             \
  bool isArray(JSContext* cx, JS::HandleObject proxy,                            \
               JS::IsArrayAnswer* answer) const override;                        \
  const char* className(JSContext* cx, JS::HandleObject proxy) const override;   \
  JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,                  \
                         bool isToSource) const override;                        \
  RegExpShared* regexp_toShared(JSContext* cx,                                   \
                                JS::HandleObject proxy) const override;          \
  bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,                   \
                        JS::MutableHandleValue vp) const override;               \
  void trace(JSTracer* trc, JSObject* proxy) const override;                     \
  void finalize(JS::GCContext* gcx, JSObject* proxy) const override;             \
  size_t objectMoved(JSObject* proxy, JSObject* old) const override;             \
  bool isCallable(JSObject* obj) const override;                                 \
  bool isConstructor(JSObject* obj) const override;                              \
  bool getElements(JSContext* cx, JS::HandleObject proxy, uint32_t begin,        \
                   uint32_t end, ElementAdder* adder) const override;            \
  bool isScripted() const override;                                              \
  bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,                         \
                                           JSContext* cx) const override;

#define DEFINE_SECURITY_WRAPPER_OPS_METHODS(name, ops, handler)                  \
template <class Base>                                                            \
bool name<Base>::finalizeInBackground(                                           \
    const JS::Value& priv) const {                                               \
  return ops->finalizeInBackground(handler, priv);                               \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::canNurseryAllocate() const {                                    \
  return ops->canNurseryAllocate(handler);                                       \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::enter(JSContext* cx,                                            \
                                          JS::HandleObject wrapper,              \
                                          JS::HandleId id, Wrapper::Action act,  \
                                          bool mayThrow, bool* bp) const {       \
  return ops->enter(handler, cx, wrapper, id, act, mayThrow, bp);                \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::getOwnPropertyDescriptor(                                       \
    JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                      \
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {      \
  return ops->getOwnPropertyDescriptor(handler, cx, proxy, id, desc);            \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::defineProperty(                                                 \
    JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                      \
    JS::Handle<JS::PropertyDescriptor> desc, JS::ObjectOpResult& result) const { \
  return ops->defineProperty(handler, cx, proxy, id, desc, result);              \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::ownPropertyKeys(                                                \
    JSContext* cx, JS::HandleObject proxy,                                       \
    JS::MutableHandleIdVector props) const {                                     \
  return ops->ownPropertyKeys(handler, cx, proxy, props);                        \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::delete_(JSContext* cx,                                          \
                                            JS::HandleObject proxy,              \
                                            JS::HandleId id,                     \
                                            JS::ObjectOpResult& result) const {  \
  return ops->delete_(handler, cx, proxy, id, result);                           \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::getPrototype(                                                   \
    JSContext* cx, JS::HandleObject proxy,                                       \
    JS::MutableHandleObject protop) const {                                      \
  return ops->getPrototype(handler, cx, proxy, protop);                          \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::setPrototype(                                                   \
    JSContext* cx, JS::HandleObject proxy, JS::HandleObject proto,               \
    JS::ObjectOpResult& result) const {                                          \
  return ops->setPrototype(handler, cx, proxy, proto, result);                   \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::getPrototypeIfOrdinary(                                         \
    JSContext* cx, JS::HandleObject proxy, bool* isOrdinary,                     \
    JS::MutableHandleObject protop) const {                                      \
  return ops->getPrototypeIfOrdinary(handler, cx, proxy, isOrdinary, protop);    \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::setImmutablePrototype(                                          \
    JSContext* cx, JS::HandleObject proxy, bool* succeeded) const {              \
  return ops->setImmutablePrototype(handler, cx, proxy, succeeded);              \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::preventExtensions(                                              \
    JSContext* cx, JS::HandleObject proxy, JS::ObjectOpResult& result) const {   \
  return ops->preventExtensions(handler, cx, proxy, result);                     \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::isExtensible(JSContext* cx,                                     \
                                                 JS::HandleObject proxy,         \
                                                 bool* extensible) const {       \
  return ops->isExtensible(handler, cx, proxy, extensible);                      \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::has(JSContext* cx, JS::HandleObject proxy,                      \
                                        JS::HandleId id, bool* bp) const {       \
  return ops->has(handler, cx, proxy, id, bp);                                   \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::get(JSContext* cx, JS::HandleObject proxy,                      \
                                        JS::HandleValue receiver,                \
                                        JS::HandleId id,                         \
                                        JS::MutableHandleValue vp) const {       \
  return ops->get(handler, cx, proxy, receiver, id, vp);                         \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::set(JSContext* cx, JS::HandleObject proxy,                      \
                                        JS::HandleId id, JS::HandleValue v,      \
                                        JS::HandleValue receiver,                \
                                        JS::ObjectOpResult& result) const {      \
  return ops->set(handler, cx, proxy, id, v, receiver, result);                  \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::useProxyExpandoObjectForPrivateFields()                         \
    const {                                                                      \
  return ops->useProxyExpandoObjectForPrivateFields(handler);                    \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::throwOnPrivateField() const {                                   \
  return ops->throwOnPrivateField(handler);                                      \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::call(JSContext* cx, JS::HandleObject proxy,                     \
                                         const JS::CallArgs& args) const {       \
  return ops->call(handler, cx, proxy, args);                                    \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::construct(JSContext* cx,                                        \
                                              JS::HandleObject proxy,            \
                                              const JS::CallArgs& args) const {  \
  return ops->construct(handler, cx, proxy, args);                               \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::enumerate(                                                      \
    JSContext* cx, JS::HandleObject proxy,                                       \
    JS::MutableHandleIdVector props) const {                                     \
  return ops->enumerate(handler, cx, proxy, props);                              \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::hasOwn(JSContext* cx,                                           \
                                           JS::HandleObject proxy,               \
                                           JS::HandleId id, bool* bp) const {    \
  return ops->hasOwn(handler, cx, proxy, id, bp);                                \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::getOwnEnumerablePropertyKeys(                                   \
    JSContext* cx, JS::HandleObject proxy,                                       \
    JS::MutableHandleIdVector props) const {                                     \
  return ops->getOwnEnumerablePropertyKeys(handler, cx, proxy, props);           \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::nativeCall(JSContext* cx,                                       \
                                               JS::IsAcceptableThis test,        \
                                               JS::NativeImpl impl,              \
                                               const JS::CallArgs& args) const { \
  return ops->nativeCall(handler, cx, test, impl, args);                         \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::getBuiltinClass(JSContext* cx,                                  \
                                                    JS::HandleObject proxy,      \
                                                    ESClass* cls) const {        \
  return ops->getBuiltinClass(handler, cx, proxy, cls);                          \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::isArray(JSContext* cx,                                          \
                                            JS::HandleObject proxy,              \
                                            JS::IsArrayAnswer* answer) const {   \
  return ops->isArray(handler, cx, proxy, answer);                               \
}                                                                                \
template <class Base>                                                            \
const char* name<Base>::className(                                               \
    JSContext* cx, JS::HandleObject proxy) const {                               \
  return ops->className(handler, cx, proxy);                                     \
}                                                                                \
template <class Base>                                                            \
JSString* name<Base>::fun_toString(JSContext* cx,                                \
                                                      JS::HandleObject proxy,    \
                                                      bool isToSource) const {   \
  return ops->fun_toString(handler, cx, proxy, isToSource);                      \
}                                                                                \
template <class Base>                                                            \
RegExpShared* name<Base>::regexp_toShared(                                       \
    JSContext* cx, JS::HandleObject proxy) const {                               \
  return ops->regexp_toShared(handler, cx, proxy);                               \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::boxedValue_unbox(                                               \
    JSContext* cx, JS::HandleObject proxy, JS::MutableHandleValue vp) const {    \
  return ops->boxedValue_unbox(handler, cx, proxy, vp);                          \
}                                                                                \
template <class Base>                                                            \
void name<Base>::trace(JSTracer* trc,                                            \
                                          JSObject* proxy) const {               \
  return ops->trace(handler, trc, proxy);                                        \
}                                                                                \
template <class Base>                                                            \
void name<Base>::finalize(JS::GCContext* gcx,                                    \
                                             JSObject* proxy) const {            \
  return ops->finalize(handler, gcx, proxy);                                     \
}                                                                                \
template <class Base>                                                            \
size_t name<Base>::objectMoved(JSObject* proxy,                                  \
                                                  JSObject* old) const {         \
  return ops->objectMoved(handler, proxy, old);                                  \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::isCallable(JSObject* obj) const {                               \
  return ops->isCallable(private_, obj);                                         \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::isConstructor(JSObject* obj) const {                            \
  return ops->isConstructor(handler, obj);                                       \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::getElements(JSContext* cx,                                      \
                                                JS::HandleObject proxy,          \
                                                uint32_t begin, uint32_t end,    \
                                                ElementAdder* adder) const {     \
  return ops->getElements(handler, cx, proxy, begin, end, adder);                \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::isScripted() const {                                            \
  return ops->isScripted(handler);                                               \
}                                                                                \
template <class Base>                                                            \
bool name<Base>::dynamicCheckedUnwrapAllowed(JS::HandleObject obj,               \
                                           JSContext* cx) const {                \
  return ops->dynamicCheckedUnwrapAllowed(handler, obj, cx);                     \
}

#define DECLARE_SECURITY_WRAPPER_OPS_CLASS(name)                             \
template <class Base>                                                        \
class JS_PUBLIC_API name : public SecurityWrapper<Base> {                    \
  const WrapperOps* pOps_;                                                   \
  const void* private_;                                                      \
public:                                                                      \
  explicit inline name(const WrapperOps* pOps, const void* priv,             \
                  unsigned flags, bool hasPrototype = false);                \
  const WrapperOps* getOps() const { return pOps_; }                         \
  const void* getPrivate() const { return private_; }                        \
  DECLARE_SECURITY_WRAPPER_OPS_METHODS                                       \
};

#define DEFINE_SECURITY_WRAPPER_OPS_CLASS(name)                               \
template <class Base>                                                         \
name<Base>::name(const WrapperOps* pOps, const void* priv,                    \
                       unsigned flags, bool hasPrototype)                     \
     : SecurityWrapper<Base>(flags, hasPrototype),                            \
       pOps_(pOps), private_(priv) {}                                         \
DEFINE_SECURITY_WRAPPER_OPS_METHODS(name, pOps_, private_)

DECLARE_SECURITY_WRAPPER_OPS_CLASS(SecurityWrapperWithOps)

typedef SecurityWrapperWithOps<CrossCompartmentWrapper>
    CrossCompartmentSecurityWrapperWithOps;
#endif

typedef SecurityWrapper<CrossCompartmentWrapper>
    CrossCompartmentSecurityWrapper;

extern JSObject* TransparentObjectWrapper(JSContext* cx,
                                          JS::HandleObject existing,
                                          JS::HandleObject obj);

inline bool IsWrapper(const JSObject* obj) {
  return IsProxy(obj) && GetProxyHandler(obj)->family() == Wrapper::family_p();
}

inline bool IsCrossCompartmentWrapper(const JSObject* obj) {
  return IsWrapper(obj) &&
         (Wrapper::wrapperHandler(obj)->flags() & Wrapper::CROSS_COMPARTMENT);
}

/* static */ inline const Wrapper* Wrapper::wrapperHandler(
    const JSObject* wrapper) {
  MOZ_ASSERT(IsWrapper(wrapper));
  return static_cast<const Wrapper*>(GetProxyHandler(wrapper));
}

// Given a JSObject, returns that object stripped of wrappers. If
// stopAtWindowProxy is true, then this returns the WindowProxy if it was
// previously wrapped. Otherwise, this returns the first object for which
// JSObject::isWrapper returns false.
//
// ExposeToActiveJS is called on wrapper targets to allow gray marking
// assertions to work while an incremental GC is in progress, but this means
// that this cannot be called from the GC or off the main thread.
JS_PUBLIC_API JSObject* UncheckedUnwrap(JSObject* obj,
                                        bool stopAtWindowProxy = true,
                                        unsigned* flagsp = nullptr);

// Given a JSObject, returns that object stripped of wrappers, except
// WindowProxy wrappers.  At each stage, the wrapper has the opportunity to veto
// the unwrap. Null is returned if there are security wrappers that can't be
// unwrapped.
//
// This does a static-only unwrap check: it basically checks whether _all_
// globals in the wrapper's source compartment should be able to access the
// wrapper target.  This won't necessarily return the right thing for the HTML
// spec's cross-origin objects (WindowProxy and Location), but is fine to use
// when failure to unwrap one of those objects wouldn't be a problem.  For
// example, if you want to test whether your target object is a specific class
// that's not WindowProxy or Location, you can use this.
//
// ExposeToActiveJS is called on wrapper targets to allow gray marking
// assertions to work while an incremental GC is in progress, but this means
// that this cannot be called from the GC or off the main thread.
JS_PUBLIC_API JSObject* CheckedUnwrapStatic(JSObject* obj);

// Unwrap only the outermost security wrapper, with the same semantics as
// above. This is the checked version of Wrapper::wrappedObject.
JS_PUBLIC_API JSObject* UnwrapOneCheckedStatic(JSObject* obj);

// Given a JSObject, returns that object stripped of wrappers. At each stage,
// the security wrapper has the opportunity to veto the unwrap. If
// stopAtWindowProxy is true, then this returns the WindowProxy if it was
// previously wrapped.  Null is returned if there are security wrappers that
// can't be unwrapped.
//
// ExposeToActiveJS is called on wrapper targets to allow gray marking
// assertions to work while an incremental GC is in progress, but this means
// that this cannot be called from the GC or off the main thread.
//
// The JSContext argument will be used for dynamic checks (needed by WindowProxy
// and Location) and should represent the Realm doing the unwrapping.  It is not
// used to throw exceptions; this function never throws.
//
// This function may be able to GC (and the static analysis definitely thinks it
// can), but it still takes a JSObject* argument, because some of its callers
// would actually have a bit of a hard time producing a Rooted.  And it ends up
// having to root internally anyway, because it wants to use the value in a loop
// and you can't assign to a HandleObject.  What this means is that callers who
// plan to use the argument object after they have called this function will
// need to root it to avoid hazard failures, even though this function doesn't
// require a Handle.
JS_PUBLIC_API JSObject* CheckedUnwrapDynamic(JSObject* obj, JSContext* cx,
                                             bool stopAtWindowProxy = true);

// Unwrap only the outermost security wrapper, with the same semantics as
// above. This is the checked version of Wrapper::wrappedObject.
JS_PUBLIC_API JSObject* UnwrapOneCheckedDynamic(JS::HandleObject obj,
                                                JSContext* cx,
                                                bool stopAtWindowProxy = true);

// Given a JSObject, returns that object stripped of wrappers. This returns the
// WindowProxy if it was previously wrapped.
//
// ExposeToActiveJS is not called on wrapper targets so this can be called from
// the GC or off the main thread.
JS_PUBLIC_API JSObject* UncheckedUnwrapWithoutExpose(JSObject* obj);

JS_PUBLIC_API void ReportAccessDenied(JSContext* cx);

JS_PUBLIC_API void NukeCrossCompartmentWrapper(JSContext* cx,
                                               JSObject* wrapper);

// If a cross-compartment wrapper source => target exists, nuke it.
JS_PUBLIC_API void NukeCrossCompartmentWrapperIfExists(JSContext* cx,
                                                       JS::Compartment* source,
                                                       JSObject* target);

void RemapWrapper(JSContext* cx, JSObject* wobj, JSObject* newTarget);
void RemapDeadWrapper(JSContext* cx, JS::HandleObject wobj,
                      JS::HandleObject newTarget);

JS_PUBLIC_API bool RemapAllWrappersForObject(JSContext* cx,
                                             JS::HandleObject oldTarget,
                                             JS::HandleObject newTarget);

// API to recompute all cross-compartment wrappers whose source and target
// match the given filters.
JS_PUBLIC_API bool RecomputeWrappers(JSContext* cx,
                                     const CompartmentFilter& sourceFilter,
                                     const CompartmentFilter& targetFilter);

} /* namespace js */

#endif /* js_Wrapper_h */
