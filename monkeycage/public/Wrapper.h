/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Wrapper_h
#define mc_Wrapper_h

#include "monkeycage/Proxy.h"
#include "js/Wrapper.h"

#ifdef JS_SANDBOX

#include "js/sandbox/Wrapper.h"
#include "mcfriendapi.h"

namespace mc {

class ForwardingProxyHandler : public BaseProxyHandler {
  DEFINE_PROXY_HANDLER_OPS_CALLBACKS(ForwardingProxyHandler)

 public:
  explicit inline ForwardingProxyHandler(const void* aProxyFamily,
                                         bool aHasPrototype = false,
                                         bool aHasSecurityPolicy = false)
      : BaseProxyHandler(js_new<js::sandbox::ForwardingProxyHandler>(
            ops(), this, aProxyFamily, aHasPrototype, aHasSecurityPolicy)) {}

  explicit inline ForwardingProxyHandler(const js::ForwardingProxyHandler* base, bool owned = true)
      : BaseProxyHandler(base, owned) {}

  inline const js::ForwardingProxyHandler* UNSAFE_getProxyHandler() const { return static_cast<const js::ForwardingProxyHandler*>(BaseProxyHandler::UNSAFE_getProxyHandler()); }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getOwnPropertyDescriptor(cx, proxy, id,
                                                               desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::ownPropertyKeys(cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::delete_(
        cx, proxy, id, result);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::enumerate(
        cx, proxy, props);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(
      JSContext* cx, JS::HandleObject proxy, bool* isOrdinary,
      JS::MutableHandleObject protop) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::setImmutablePrototype(cx, proxy,
                                                            succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::preventExtensions(cx, proxy, result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::has(cx, proxy,
                                                                     id, bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::get(
        cx, proxy, receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::set(
        cx, proxy, id, v, receiver, result);
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::call(cx, proxy,
                                                                      args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::construct(
        cx, proxy, args);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::hasOwn(
        cx, proxy, id, bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                   props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getBuiltinClass(cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::isArray(
        cx, proxy, answer);
  }

  virtual const char* className(JSContext* cx,
                                JS::HandleObject proxy) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::className(
        cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(
      JSContext* cx, JS::HandleObject proxy) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::regexp_toShared(cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::boxedValue_unbox(cx, proxy, vp);
  }

  virtual bool isCallable(JSObject* obj) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::isCallable(
        obj);
  }

  virtual bool isConstructor(JSObject* obj) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::isConstructor(
        obj);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const override {
    return false;
  }
};

#define DEFINE_WRAPPER_OPS_CALLBACKS(ExternalWrapper)                                        \
 static bool finalizeInBackgroundCb(const void* p, const JS::Value& priv) {                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->finalizeInBackground(priv);                                                     \
 }                                                                                           \
 static bool canNurseryAllocateCb(const void* p) {                                           \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->canNurseryAllocate();                                                           \
 }                                                                                           \
 static bool enterCb(const void* p, JSContext* cx, JS::HandleObject wrapper,                 \
                     JS::HandleId id, BaseProxyHandler::Action act,                          \
                     bool mayThrow, bool* bp) {                                              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->enter(cx, wrapper, id, act, mayThrow, bp);                                      \
 }                                                                                           \
 static bool getOwnPropertyDescriptorCb(                                                     \
     const void* p, JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                  \
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {                       \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->getOwnPropertyDescriptor(cx, proxy, id, desc);                                  \
 }                                                                                           \
 static bool definePropertyCb(const void* p, JSContext* cx,                                  \
                              JS::HandleObject proxy, JS::HandleId id,                       \
                              JS::Handle<JS::PropertyDescriptor> desc,                       \
                              JS::ObjectOpResult& result) {                                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->defineProperty(cx, proxy, id, desc, result);                                    \
 }                                                                                           \
 static bool ownPropertyKeysCb(const void* p, JSContext* cx,                                 \
                               JS::HandleObject proxy,                                       \
                               JS::MutableHandleIdVector props) {                            \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->ownPropertyKeys(cx, proxy, props);                                              \
 }                                                                                           \
 static bool delete_Cb(const void* p, JSContext* cx, JS::HandleObject proxy,                 \
                       JS::HandleId id, JS::ObjectOpResult& result) {                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->delete_(cx, proxy, id, result);                                                 \
 }                                                                                           \
 static bool getPrototypeCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy,                                          \
                            JS::MutableHandleObject protop) {                                \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->getPrototype(cx, proxy, protop);                                                \
 }                                                                                           \
 static bool setPrototypeCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy, JS::HandleObject proto,                  \
                            JS::ObjectOpResult& result) {                                    \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->setPrototype(cx, proxy, proto, result);                                         \
 }                                                                                           \
 static bool getPrototypeIfOrdinaryCb(const void* p, JSContext* cx,                          \
                                      JS::HandleObject proxy, bool* isOrdinary,              \
                                      JS::MutableHandleObject protop) {                      \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->getPrototypeIfOrdinary(cx, proxy, isOrdinary, protop);                          \
 }                                                                                           \
 static bool setImmutablePrototypeCb(const void* p, JSContext* cx,                           \
                                     JS::HandleObject proxy, bool* succeeded) {              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->setImmutablePrototype(cx, proxy, succeeded);                                    \
 }                                                                                           \
 static bool preventExtensionsCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy,                                     \
                                 JS::ObjectOpResult& result) {                               \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->preventExtensions(cx, proxy, result);                                           \
 }                                                                                           \
 static bool isExtensibleCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy, bool* extensible) {                      \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->isExtensible(cx, proxy, extensible);                                            \
 }                                                                                           \
 static bool hasCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleId id, bool* bp) {                                              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->has(cx, proxy, id, bp);                                                         \
 }                                                                                           \
 static bool getCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleValue receiver, JS::HandleId id,                                \
                   JS::MutableHandleValue vp) {                                              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->get(cx, proxy, receiver, id, vp);                                               \
 }                                                                                           \
 static bool setCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleId id, JS::HandleValue v, JS::HandleValue receiver,             \
                   JS::ObjectOpResult& result) {                                             \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->set(cx, proxy, id, v, receiver, result);                                        \
 }                                                                                           \
 static bool useProxyExpandoObjectForPrivateFieldsCb(const void* p) {                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->useProxyExpandoObjectForPrivateFields();                                        \
 }                                                                                           \
 static bool throwOnPrivateFieldCb(const void* p) {                                          \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->throwOnPrivateField();                                                          \
 }                                                                                           \
 static bool callCb(const void* p, JSContext* cx, JS::HandleObject proxy,                    \
                    const JS::CallArgs& args) {                                              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->call(cx, proxy, args);                                                          \
 }                                                                                           \
 static bool constructCb(const void* p, JSContext* cx, JS::HandleObject proxy,               \
                         const JS::CallArgs& args) {                                         \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->construct(cx, proxy, args);                                                     \
 }                                                                                           \
 static bool enumerateCb(const void* p, JSContext* cx, JS::HandleObject proxy,               \
                         JS::MutableHandleIdVector props) {                                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->enumerate(cx, proxy, props);                                                    \
 }                                                                                           \
 static bool hasOwnCb(const void* p, JSContext* cx, JS::HandleObject proxy,                  \
                      JS::HandleId id, bool* bp) {                                           \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->hasOwn(cx, proxy, id, bp);                                                      \
 }                                                                                           \
 static bool getOwnEnumerablePropertyKeysCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy,                          \
                                            JS::MutableHandleIdVector props) {               \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->getOwnEnumerablePropertyKeys(cx, proxy, props);                                 \
 }                                                                                           \
 static bool nativeCallCb(const void* p, JSContext* cx,                                      \
                          JS::IsAcceptableThis test, JS::NativeImpl impl,                    \
                          const JS::CallArgs& args) {                                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->nativeCall(cx, test, impl, args);                                               \
 }                                                                                           \
 static bool getBuiltinClassCb(const void* p, JSContext* cx,                                 \
                               JS::HandleObject proxy, js::ESClass* cls) {                   \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->getBuiltinClass(cx, proxy, cls);                                                \
 }                                                                                           \
 static bool isArrayCb(const void* p, JSContext* cx, JS::HandleObject proxy,                 \
                       JS::IsArrayAnswer* answer) {                                          \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->isArray(cx, proxy, answer);                                                     \
 }                                                                                           \
 static const char* classNameCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy) {                                    \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->className(cx, proxy);                                                           \
 }                                                                                           \
 static JSString* fun_toStringCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy, bool isToSource) {                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->fun_toString(cx, proxy, isToSource);                                            \
 }                                                                                           \
 static js::RegExpShared* regexp_toSharedCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy) {                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->regexp_toShared(cx, proxy);                                                     \
 }                                                                                           \
 static bool boxedValue_unboxCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy,                                      \
                                JS::MutableHandleValue vp) {                                 \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->boxedValue_unbox(cx, proxy, vp);                                                \
 }                                                                                           \
 static void traceCb(const void* p, JSTracer* trc, JSObject* proxy) {                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->trace(trc, proxy);                                                              \
 }                                                                                           \
 static void finalizeCb(const void* p, JS::GCContext* gcx, JSObject* proxy) {                \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->finalize(gcx, proxy);                                                           \
 }                                                                                           \
 static size_t objectMovedCb(const void* p, JSObject* proxy, JSObject* old) {                \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->objectMoved(proxy, old);                                                        \
 }                                                                                           \
 static bool isCallableCb(const void* p, JSObject* obj) {                                    \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->isCallable(obj);                                                                \
 }                                                                                           \
 static bool isConstructorCb(const void* p, JSObject* obj) {                                 \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->isConstructor(obj);                                                             \
 }                                                                                           \
 static bool getElementsCb(const void* p, JSContext* cx, JS::HandleObject proxy,             \
                           uint32_t begin, uint32_t end,                                     \
                           js::ElementAdder* adder) {                                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->getElements(cx, proxy, begin, end, adder);                                      \
 }                                                                                           \
 static bool isScriptedCb(const void* p) {                                                   \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->isScripted();                                                                   \
 }                                                                                           \
 static bool dynamicCheckedUnwrapAllowedCb(const void* p, JS::HandleObject obj,              \
                                           JSContext* cx) {                                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   return h->dynamicCheckedUnwrapAllowed(obj, cx);                                           \
 }                                                                                           \
 static const js::sandbox::WrapperOps* ops() {                                               \
   static const js::sandbox::WrapperOps __ops = {                                            \
       {                                                                                     \
       .finalizeInBackground = MC::Sandbox::RegisterCallback(finalizeInBackgroundCb).UNSAFE_get(),                                       \
       .canNurseryAllocate = MC::Sandbox::RegisterCallback(canNurseryAllocateCb).UNSAFE_get(),                                           \
       .enter = MC::Sandbox::RegisterCallback(enterCb).UNSAFE_get(),                                                                     \
       .getOwnPropertyDescriptor = MC::Sandbox::RegisterCallback(getOwnPropertyDescriptorCb).UNSAFE_get(),                               \
       .defineProperty = MC::Sandbox::RegisterCallback(definePropertyCb).UNSAFE_get(),                                                   \
       .ownPropertyKeys = MC::Sandbox::RegisterCallback(ownPropertyKeysCb).UNSAFE_get(),                                                 \
       .delete_ = MC::Sandbox::RegisterCallback(delete_Cb).UNSAFE_get(),                                                                 \
       .getPrototype = MC::Sandbox::RegisterCallback(getPrototypeCb).UNSAFE_get(),                                                       \
       .setPrototype = MC::Sandbox::RegisterCallback(setPrototypeCb).UNSAFE_get(),                                                       \
       .getPrototypeIfOrdinary = MC::Sandbox::RegisterCallback(getPrototypeIfOrdinaryCb).UNSAFE_get(),                                   \
       .setImmutablePrototype = MC::Sandbox::RegisterCallback(setImmutablePrototypeCb).UNSAFE_get(),                                     \
       .preventExtensions = MC::Sandbox::RegisterCallback(preventExtensionsCb).UNSAFE_get(),                                             \
       .isExtensible = MC::Sandbox::RegisterCallback(isExtensibleCb).UNSAFE_get(),                                                       \
       .has = MC::Sandbox::RegisterCallback(hasCb).UNSAFE_get(),                                                                         \
       .get = MC::Sandbox::RegisterCallback(getCb).UNSAFE_get(),                                                                         \
       .set = MC::Sandbox::RegisterCallback(setCb).UNSAFE_get(),                                                                         \
       .useProxyExpandoObjectForPrivateFields = MC::Sandbox::RegisterCallback(useProxyExpandoObjectForPrivateFieldsCb).UNSAFE_get(),     \
       .throwOnPrivateField = MC::Sandbox::RegisterCallback(throwOnPrivateFieldCb).UNSAFE_get(),                                         \
       .call = MC::Sandbox::RegisterCallback(callCb).UNSAFE_get(),                                                                       \
       .construct = MC::Sandbox::RegisterCallback(constructCb).UNSAFE_get(),                                                             \
       .enumerate = MC::Sandbox::RegisterCallback(enumerateCb).UNSAFE_get(),                                                             \
       .hasOwn = MC::Sandbox::RegisterCallback(hasOwnCb).UNSAFE_get(),                                                                   \
       .getOwnEnumerablePropertyKeys = MC::Sandbox::RegisterCallback(getOwnEnumerablePropertyKeysCb).UNSAFE_get(),                       \
       .nativeCall = MC::Sandbox::RegisterCallback(nativeCallCb).UNSAFE_get(),                                                           \
       .getBuiltinClass = MC::Sandbox::RegisterCallback(getBuiltinClassCb).UNSAFE_get(),                                                 \
       .isArray = MC::Sandbox::RegisterCallback(isArrayCb).UNSAFE_get(),                                                                 \
       .className = MC::Sandbox::RegisterCallback(classNameCb).UNSAFE_get(),                                                             \
       .fun_toString = MC::Sandbox::RegisterCallback(fun_toStringCb).UNSAFE_get(),                                                       \
       .regexp_toShared = MC::Sandbox::RegisterCallback(regexp_toSharedCb).UNSAFE_get(),                                                 \
       .boxedValue_unbox = MC::Sandbox::RegisterCallback(boxedValue_unboxCb).UNSAFE_get(),                                               \
       .trace = MC::Sandbox::RegisterCallback(traceCb).UNSAFE_get(),                                                                     \
       .finalize = MC::Sandbox::RegisterCallback(finalizeCb).UNSAFE_get(),                                                               \
       .objectMoved = MC::Sandbox::RegisterCallback(objectMovedCb).UNSAFE_get(),                                                         \
       .isCallable = MC::Sandbox::RegisterCallback(isCallableCb).UNSAFE_get(),                                                           \
       .isConstructor = MC::Sandbox::RegisterCallback(isConstructorCb).UNSAFE_get(),                                                     \
       .getElements = MC::Sandbox::RegisterCallback(getElementsCb).UNSAFE_get(),                                                         \
       .isScripted = MC::Sandbox::RegisterCallback(isScriptedCb).UNSAFE_get(),                                                           \
       },                                                                                                                                \
       MC::Sandbox::RegisterCallback(dynamicCheckedUnwrapAllowedCb).UNSAFE_get(),                                                        \
   };                                                                                                                                    \
   return &__ops;                                                                                                                        \
 }

class Wrapper : public ForwardingProxyHandler {
  DEFINE_WRAPPER_OPS_CALLBACKS(Wrapper)

 public:
  using Flags = js::Wrapper::Flags;
  using BaseProxyHandler::Action;
  using Unsafe = js::Wrapper;

  explicit inline Wrapper(unsigned aFlags, bool aHasPrototype = false,
                          bool aHasSecurityPolicy = false)
      : ForwardingProxyHandler(js_new<js::sandbox::Wrapper>(
            ops(), this, aFlags, aHasPrototype, aHasSecurityPolicy)) {}

  explicit inline Wrapper(const js::Wrapper* base, bool owned = true)
      : ForwardingProxyHandler(base, owned) {}

  inline const js::Wrapper* UNSAFE_getWrapper() const {
    return static_cast<const js::Wrapper*>(
        ForwardingProxyHandler::UNSAFE_getProxyHandler());
  }

  virtual bool finalizeInBackground(const JS::Value& priv) const override {
    return UNSAFE_getWrapper()->js::Wrapper::finalizeInBackground(priv);
  }

  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,
                                           JSContext* cx) const {
    return UNSAFE_getWrapper()->js::Wrapper::dynamicCheckedUnwrapAllowed(obj, cx);
  }

  static inline JSObject* New(JSContext* cx, JSObject* obj, const Wrapper* handler,
                       const js::WrapperOptions& options = js::WrapperOptions()) {
    return js::Wrapper::New(cx, obj, handler->UNSAFE_getWrapper(), options);
  }

  static JSObject* Renew(JSObject* existing, JSObject* obj,
                         const Wrapper* handler) {
    return js::Wrapper::Renew(existing, obj, handler->UNSAFE_getWrapper());
  }

  static inline JSObject* wrappedObject(JSObject* wrapper) {
    return js::Wrapper::wrappedObject(wrapper);
  }

  unsigned flags() const { return UNSAFE_getWrapper()->flags(); }

  bool isCrossCompartmentWrapper() const {
    return flags() & Flags::CROSS_COMPARTMENT;
  }

  static const char* getFamily();
  static const Wrapper* getSingleton();
  static const Wrapper* getSingletonWithPrototype();
};

class CrossCompartmentWrapper : public Wrapper {
  DEFINE_WRAPPER_OPS_CALLBACKS(CrossCompartmentWrapper)

public:
 using Unsafe = js::CrossCompartmentWrapper;

 explicit inline CrossCompartmentWrapper(unsigned aFlags,
                                         bool aHasPrototype = false,
                                         bool aHasSecurityPolicy = false)
     : Wrapper(js_new<js::sandbox::CrossCompartmentWrapper>(
           ops(), this, aFlags, aHasPrototype, aHasSecurityPolicy)) {}

 explicit inline CrossCompartmentWrapper(
     const js::CrossCompartmentWrapper* base, bool owned = true)
     : Wrapper(base, owned) {}

 inline const js::CrossCompartmentWrapper* UNSAFE_getWrapper() const {
   return static_cast<const js::CrossCompartmentWrapper*>(
       Wrapper::UNSAFE_getWrapper());
 }

 virtual bool getOwnPropertyDescriptor(
     JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
     const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::getOwnPropertyDescriptor(cx, proxy, id,
                                                               desc);
 }

 virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                             JS::HandleId id,
                             JS::Handle<JS::PropertyDescriptor> desc,
                             JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::defineProperty(
       cx, proxy, id, desc, result);
 }

 virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                              JS::MutableHandleIdVector props) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::ownPropertyKeys(
       cx, proxy, props);
 }

 virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::delete_(cx, proxy,
                                                                    id, result);
 }

 virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                        JS::MutableHandleIdVector props) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::enumerate(cx, proxy,
                                                                      props);
 }

 virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                           JS::MutableHandleObject protop) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::getPrototype(
       cx, proxy, protop);
 }

 virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                           JS::HandleObject proto,
                           JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::setPrototype(
       cx, proxy, proto, result);
 }

 virtual bool getPrototypeIfOrdinary(
     JSContext* cx, JS::HandleObject proxy, bool* isOrdinary,
     JS::MutableHandleObject protop) const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::getPrototypeIfOrdinary(
           cx, proxy, isOrdinary, protop);
 }

 virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                    bool* succeeded) const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::setImmutablePrototype(cx, proxy,
                                                            succeeded);
 }

 virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::preventExtensions(
       cx, proxy, result);
 }

 virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                           bool* extensible) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::isExtensible(
       cx, proxy, extensible);
 }

 virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                  bool* bp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::has(cx, proxy, id,
                                                                bp);
 }

 virtual bool get(JSContext* cx, JS::HandleObject proxy,
                  JS::HandleValue receiver, JS::HandleId id,
                  JS::MutableHandleValue vp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::get(
       cx, proxy, receiver, id, vp);
 }

 virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                  JS::HandleValue v, JS::HandleValue receiver,
                  JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::set(
       cx, proxy, id, v, receiver, result);
 }

 virtual bool call(JSContext* cx, JS::HandleObject proxy,
                   const JS::CallArgs& args) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::call(cx, proxy,
                                                                 args);
 }

 virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                        const JS::CallArgs& args) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::construct(cx, proxy,
                                                                      args);
 }

 virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                     bool* bp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::hasOwn(cx, proxy,
                                                                   id, bp);
 }

 virtual bool getOwnEnumerablePropertyKeys(
     JSContext* cx, JS::HandleObject proxy,
     JS::MutableHandleIdVector props) const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                   props);
 }

 virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                         JS::NativeImpl impl,
                         const JS::CallArgs& args) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::nativeCall(
       cx, test, impl, args);
 }

 virtual const char* className(JSContext* cx,
                               JS::HandleObject proxy) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::className(cx,
                                                                      proxy);
 }

 virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                bool isToSource) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::fun_toString(
       cx, proxy, isToSource);
 }

 virtual js::RegExpShared* regexp_toShared(
     JSContext* cx, JS::HandleObject proxy) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::regexp_toShared(
       cx, proxy);
 }

 virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleValue vp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::boxedValue_unbox(
       cx, proxy, vp);
 }

 virtual bool canNurseryAllocate() const override { return true; }

 static const CrossCompartmentWrapper* getSingleton();
};

class OpaqueCrossCompartmentWrapper : public CrossCompartmentWrapper {
  DEFINE_WRAPPER_OPS_CALLBACKS(OpaqueCrossCompartmentWrapper)

 public:
  explicit inline OpaqueCrossCompartmentWrapper()
      : CrossCompartmentWrapper(
            js_new<js::sandbox::OpaqueCrossCompartmentWrapper>(ops(), this)) {}

  explicit inline OpaqueCrossCompartmentWrapper(
      const js::OpaqueCrossCompartmentWrapper* base, bool owned = true)
      : CrossCompartmentWrapper(base, owned) {}

  inline const js::OpaqueCrossCompartmentWrapper* UNSAFE_getWrapper() const {
    return static_cast<const js::OpaqueCrossCompartmentWrapper*>(
        CrossCompartmentWrapper::UNSAFE_getWrapper());
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getOwnPropertyDescriptor(cx, proxy, id,
                                                                desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::ownPropertyKeys(
        cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::delete_(
        cx, proxy, id, result);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::enumerate(
        cx, proxy, props);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(
      JSContext* cx, JS::HandleObject proxy, bool* isOrdinary,
      JS::MutableHandleObject protop) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::setImmutablePrototype(cx, proxy,
                                                             succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::preventExtensions(
        cx, proxy, result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::has(cx, proxy, id,
                                                                 bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::get(
        cx, proxy, receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::set(
        cx, proxy, id, v, receiver, result);
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::call(cx, proxy,
                                                                  args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::construct(
        cx, proxy, args);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::hasOwn(cx, proxy,
                                                                    id, bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                    props);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getBuiltinClass(cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::isArray(
        cx, proxy, answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::className(
        cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::fun_toString(
        cx, proxy, isToSource);
  }

  static const OpaqueCrossCompartmentWrapper* getSingleton();
};

template <class Base>
class SecurityWrapper : public Base {
  DEFINE_WRAPPER_OPS_CALLBACKS(SecurityWrapper<Base>)

 public:
  typedef Base Permissive;
  typedef SecurityWrapper<Base> Restrictive;
  typedef js::SecurityWrapper<typename Base::Unsafe> Unsafe;

  explicit inline SecurityWrapper(unsigned flags, bool hasPrototype = false)
      : Base(js_new<js::sandbox::SecurityWrapper<typename Base::Unsafe>>(
            ops(), this, flags, hasPrototype)) {}

  inline const Unsafe* UNSAFE_getWrapper() const {
    return static_cast<const Unsafe*>(Base::UNSAFE_getWrapper());
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const override {
    return UNSAFE_getWrapper()->Unsafe::enter(cx, wrapper, id, act, mayThrow, bp);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->Unsafe::defineProperty(cx, proxy, id, desc,
                                                     result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override {
    return UNSAFE_getWrapper()->Unsafe::isExtensible(cx, proxy, extensible);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->Unsafe::preventExtensions(cx, proxy, result);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->Unsafe::setPrototype(cx, proxy, proto, result);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override {
    return UNSAFE_getWrapper()->Unsafe::setImmutablePrototype(cx, proxy,
                                                            succeeded);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const override {
    return UNSAFE_getWrapper()->Unsafe::nativeCall(cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const override {
    return UNSAFE_getWrapper()->Unsafe::getBuiltinClass(cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override {
    return UNSAFE_getWrapper()->Unsafe::isArray(cx, proxy, answer);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const override {
    return UNSAFE_getWrapper()->Unsafe::regexp_toShared(cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override {
    return UNSAFE_getWrapper()->Unsafe::boxedValue_unbox(cx, proxy, vp);
  }
};

typedef SecurityWrapper<CrossCompartmentWrapper> CrossCompartmentSecurityWrapper;

inline bool IsWrapper(const JSObject* obj) {
  return js::IsProxy(obj) &&
         mc::GetProxyHandlerFamily(obj) == mc::Wrapper::getFamily();
}

inline bool IsWrapperHandler(const JSObject* obj, const Wrapper* wrapper) {
  MOZ_ASSERT(IsWrapper(obj));
  return static_cast<const js::Wrapper*>(js::GetProxyHandler(obj)) == wrapper->UNSAFE_getWrapper();
}

inline bool IsCrossCompartmentWrapper(const JSObject* obj) {
  return IsWrapper(obj) &&
         (static_cast<const js::Wrapper*>(js::GetProxyHandler(obj))->flags() & js::Wrapper::CROSS_COMPARTMENT);
}

inline JSObject* CheckedUnwrapDynamic(JSObject* obj, MCContext* cx,
                                             bool stopAtWindowProxy = true) {
    return js::CheckedUnwrapDynamic(obj, cx->cx_, stopAtWindowProxy);
}
}

namespace js {

inline bool RecomputeWrappers(JSContext* cx,
                              const mc::CompartmentFilter& sourceFilter,
                              const mc::CompartmentFilter& targetFilter) {
  return RecomputeWrappers(cx, *sourceFilter.inner_, *targetFilter.inner_);
}
}
#else
namespace mc {

using ForwardingProxyHandler = js::ForwardingProxyHandler;
using Wrapper = js::Wrapper;
using CrossCompartmentWrapper = js::CrossCompartmentWrapper;
using OpaqueCrossCompartmentWrapper = js::OpaqueCrossCompartmentWrapper;

template <class Base>
using SecurityWrapper = js::SecurityWrapper<Base>;
using CrossCompartmentSecurityWrapper = js::CrossCompartmentSecurityWrapper;

inline bool IsWrapper(const JSObject* obj) {
  return js::IsWrapper(obj);
}

inline bool IsCrossCompartmentWrapper(const JSObject* obj) {
  return js::IsCrossCompartmentWrapper(obj);
}
 
}
#endif

#endif
