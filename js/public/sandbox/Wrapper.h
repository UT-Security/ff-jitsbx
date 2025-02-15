/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Wrapper_h
#define js_sandbox_Wrapper_h

#include "js/Wrapper.h"
#ifdef JS_SANDBOX_API
#include "js/sandbox/Proxy.h"
#include "js/sandbox/sobox.h"
#endif

namespace js {
namespace sandbox {
#ifdef JS_SANDBOX_API

class ForwardingProxyHandler {
private:
  js::ForwardingProxyHandlerWithOps base_;
public:
  DEFINE_PROXY_HANDLER_OPS_CALLBACKS(ForwardingProxyHandler)
  
 explicit inline ForwardingProxyHandler(const void* aProxyFamily,
                                  bool aHasPrototype = false,
                                  bool aHasSecurityPolicy = false)
     : base_(ops(), this, aProxyFamily, aHasPrototype, aHasSecurityPolicy) {}

  bool hasPrototype() const { return base_.hasPrototype(); }
  bool hasSecurityPolicy() const { return base_.hasSecurityPolicy(); }
  inline const void* family() const { return base_.family(); }
  inline const js::ForwardingProxyHandlerWithOps* getProxyHandler() const { return &base_; }

  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return getProxyHandler()->js::ForwardingProxyHandler::finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const {
    return getProxyHandler()->js::ForwardingProxyHandler::canNurseryAllocate();
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return getProxyHandler()->js::ForwardingProxyHandler::enter(cx, wrapper, id, act,
                                                          mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {
    return getProxyHandler()
        ->js::ForwardingProxyHandler::getOwnPropertyDescriptor(cx, proxy, id,
                                                               desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::ForwardingProxyHandler::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::ForwardingProxyHandler::ownPropertyKeys(
        cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::ForwardingProxyHandler::delete_(cx, proxy, id,
                                                                  result);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return getProxyHandler()->js::ForwardingProxyHandler::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::ForwardingProxyHandler::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const {
    return getProxyHandler()
        ->js::ForwardingProxyHandler::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return getProxyHandler()->js::ForwardingProxyHandler::setImmutablePrototype(
        cx, proxy, succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::ForwardingProxyHandler::preventExtensions(
        cx, proxy, result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const {
    return getProxyHandler()->js::ForwardingProxyHandler::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return getProxyHandler()->js::ForwardingProxyHandler::has(cx, proxy, id,
                                                              bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::ForwardingProxyHandler::get(cx, proxy,
                                                              receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::ForwardingProxyHandler::set(cx, proxy, id, v,
                                                              receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const { return false; }

  virtual bool throwOnPrivateField() const {
    return getProxyHandler()->js::ForwardingProxyHandler::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return getProxyHandler()->js::ForwardingProxyHandler::call(cx, proxy, args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return getProxyHandler()->js::ForwardingProxyHandler::construct(cx, proxy,
                                                                    args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::ForwardingProxyHandler::enumerate(cx, proxy,
                                                                    props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return getProxyHandler()->js::ForwardingProxyHandler::hasOwn(cx, proxy, id,
                                                                 bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->js::ForwardingProxyHandler::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                   props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return getProxyHandler()->js::ForwardingProxyHandler::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return getProxyHandler()->js::ForwardingProxyHandler::getBuiltinClass(
        cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return getProxyHandler()->js::ForwardingProxyHandler::isArray(cx, proxy,
                                                                  answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return getProxyHandler()->js::ForwardingProxyHandler::className(cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return getProxyHandler()->js::ForwardingProxyHandler::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return getProxyHandler()->js::ForwardingProxyHandler::regexp_toShared(
        cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::ForwardingProxyHandler::boxedValue_unbox(
        cx, proxy, vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return getProxyHandler()->js::ForwardingProxyHandler::trace(trc, proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return getProxyHandler()->js::ForwardingProxyHandler::finalize(gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return getProxyHandler()->js::ForwardingProxyHandler::objectMoved(proxy, old);
  }
  virtual bool isCallable(JSObject* obj) const {
    return getProxyHandler()->js::ForwardingProxyHandler::isCallable(obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return getProxyHandler()->js::ForwardingProxyHandler::isConstructor(obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return getProxyHandler()->js::ForwardingProxyHandler::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return getProxyHandler()->js::ForwardingProxyHandler::isScripted();
  }
};

inline const js::BaseProxyHandler* GetProxyHandler(const ForwardingProxyHandler* handler) {
  return handler->getProxyHandler();
}


#define DEFINE_WRAPPER_OPS_CALLBACKS(ExternalProxyHandler)                            \
 static bool finalizeInBackgroundCb(const void* p, const JS::Value& priv) {           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->finalizeInBackground(priv);                                              \
 }                                                                                    \
 static bool canNurseryAllocateCb(const void* p) {                                    \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->canNurseryAllocate();                                                    \
 }                                                                                    \
 static bool enterCb(const void* p, JSContext* cx, JS::HandleObject wrapper,          \
                     JS::HandleId id, js::BaseProxyHandler::Action act,               \
                     bool mayThrow, bool* bp) {                                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->enter(cx, wrapper, id, act, mayThrow, bp);                               \
 }                                                                                    \
 static bool getOwnPropertyDescriptorCb(                                              \
     const void* p, JSContext* cx, JS::HandleObject proxy, JS::HandleId id,           \
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {                \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->getOwnPropertyDescriptor(cx, proxy, id, desc);                           \
 }                                                                                    \
 static bool definePropertyCb(const void* p, JSContext* cx,                           \
                              JS::HandleObject proxy, JS::HandleId id,                \
                              JS::Handle<JS::PropertyDescriptor> desc,                \
                              JS::ObjectOpResult& result) {                           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->defineProperty(cx, proxy, id, desc, result);                             \
 }                                                                                    \
 static bool ownPropertyKeysCb(const void* p, JSContext* cx,                          \
                               JS::HandleObject proxy,                                \
                               JS::MutableHandleIdVector props) {                     \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->ownPropertyKeys(cx, proxy, props);                                       \
 }                                                                                    \
 static bool delete_Cb(const void* p, JSContext* cx, JS::HandleObject proxy,          \
                       JS::HandleId id, JS::ObjectOpResult& result) {                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->delete_(cx, proxy, id, result);                                          \
 }                                                                                    \
 static bool getPrototypeCb(const void* p, JSContext* cx,                             \
                            JS::HandleObject proxy,                                   \
                            JS::MutableHandleObject protop) {                         \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->getPrototype(cx, proxy, protop);                                         \
 }                                                                                    \
 static bool setPrototypeCb(const void* p, JSContext* cx,                             \
                            JS::HandleObject proxy, JS::HandleObject proto,           \
                            JS::ObjectOpResult& result) {                             \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->setPrototype(cx, proxy, proto, result);                                  \
 }                                                                                    \
 static bool getPrototypeIfOrdinaryCb(const void* p, JSContext* cx,                   \
                                      JS::HandleObject proxy, bool* isOrdinary,       \
                                      JS::MutableHandleObject protop) {               \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->getPrototypeIfOrdinary(cx, proxy, isOrdinary, protop);                   \
 }                                                                                    \
 static bool setImmutablePrototypeCb(const void* p, JSContext* cx,                    \
                                     JS::HandleObject proxy, bool* succeeded) {       \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->setImmutablePrototype(cx, proxy, succeeded);                             \
 }                                                                                    \
 static bool preventExtensionsCb(const void* p, JSContext* cx,                        \
                                 JS::HandleObject proxy,                              \
                                 JS::ObjectOpResult& result) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->preventExtensions(cx, proxy, result);                                    \
 }                                                                                    \
 static bool isExtensibleCb(const void* p, JSContext* cx,                             \
                            JS::HandleObject proxy, bool* extensible) {               \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->isExtensible(cx, proxy, extensible);                                     \
 }                                                                                    \
 static bool hasCb(const void* p, JSContext* cx, JS::HandleObject proxy,              \
                   JS::HandleId id, bool* bp) {                                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->has(cx, proxy, id, bp);                                                  \
 }                                                                                    \
 static bool getCb(const void* p, JSContext* cx, JS::HandleObject proxy,              \
                   JS::HandleValue receiver, JS::HandleId id,                         \
                   JS::MutableHandleValue vp) {                                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->get(cx, proxy, receiver, id, vp);                                        \
 }                                                                                    \
 static bool setCb(const void* p, JSContext* cx, JS::HandleObject proxy,              \
                   JS::HandleId id, JS::HandleValue v, JS::HandleValue receiver,      \
                   JS::ObjectOpResult& result) {                                      \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->set(cx, proxy, id, v, receiver, result);                                 \
 }                                                                                    \
 static bool useProxyExpandoObjectForPrivateFieldsCb(const void* p) {                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->useProxyExpandoObjectForPrivateFields();                                 \
 }                                                                                    \
 static bool throwOnPrivateFieldCb(const void* p) {                                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->throwOnPrivateField();                                                   \
 }                                                                                    \
 static bool callCb(const void* p, JSContext* cx, JS::HandleObject proxy,             \
                    const JS::CallArgs& args) {                                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->call(cx, proxy, args);                                                   \
 }                                                                                    \
 static bool constructCb(const void* p, JSContext* cx, JS::HandleObject proxy,        \
                         const JS::CallArgs& args) {                                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->construct(cx, proxy, args);                                              \
 }                                                                                    \
 static bool enumerateCb(const void* p, JSContext* cx, JS::HandleObject proxy,        \
                         JS::MutableHandleIdVector props) {                           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->enumerate(cx, proxy, props);                                             \
 }                                                                                    \
 static bool hasOwnCb(const void* p, JSContext* cx, JS::HandleObject proxy,           \
                      JS::HandleId id, bool* bp) {                                    \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->hasOwn(cx, proxy, id, bp);                                               \
 }                                                                                    \
 static bool getOwnEnumerablePropertyKeysCb(const void* p, JSContext* cx,             \
                                            JS::HandleObject proxy,                   \
                                            JS::MutableHandleIdVector props) {        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->getOwnEnumerablePropertyKeys(cx, proxy, props);                          \
 }                                                                                    \
 static bool nativeCallCb(const void* p, JSContext* cx,                               \
                          JS::IsAcceptableThis test, JS::NativeImpl impl,             \
                          const JS::CallArgs& args) {                                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->nativeCall(cx, test, impl, args);                                        \
 }                                                                                    \
 static bool getBuiltinClassCb(const void* p, JSContext* cx,                          \
                               JS::HandleObject proxy, js::ESClass* cls) {            \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->getBuiltinClass(cx, proxy, cls);                                         \
 }                                                                                    \
 static bool isArrayCb(const void* p, JSContext* cx, JS::HandleObject proxy,          \
                       JS::IsArrayAnswer* answer) {                                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->isArray(cx, proxy, answer);                                              \
 }                                                                                    \
 static const char* classNameCb(const void* p, JSContext* cx,                         \
                                JS::HandleObject proxy) {                             \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->className(cx, proxy);                                                    \
 }                                                                                    \
 static JSString* fun_toStringCb(const void* p, JSContext* cx,                        \
                                 JS::HandleObject proxy, bool isToSource) {           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->fun_toString(cx, proxy, isToSource);                                     \
 }                                                                                    \
 static js::RegExpShared* regexp_toSharedCb(const void* p, JSContext* cx,             \
                                            JS::HandleObject proxy) {                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->regexp_toShared(cx, proxy);                                              \
 }                                                                                    \
 static bool boxedValue_unboxCb(const void* p, JSContext* cx,                         \
                                JS::HandleObject proxy,                               \
                                JS::MutableHandleValue vp) {                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->boxedValue_unbox(cx, proxy, vp);                                         \
 }                                                                                    \
 static void traceCb(const void* p, JSTracer* trc, JSObject* proxy) {                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->trace(trc, proxy);                                                       \
 }                                                                                    \
 static void finalizeCb(const void* p, JS::GCContext* gcx, JSObject* proxy) {         \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->finalize(gcx, proxy);                                                    \
 }                                                                                    \
 static size_t objectMovedCb(const void* p, JSObject* proxy, JSObject* old) {         \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->objectMoved(proxy, old);                                                 \
 }                                                                                    \
 static bool isCallableCb(const void* p, JSObject* obj) {                             \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->isCallable(obj);                                                         \
 }                                                                                    \
 static bool isConstructorCb(const void* p, JSObject* obj) {                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->isConstructor(obj);                                                      \
 }                                                                                    \
 static bool getElementsCb(const void* p, JSContext* cx, JS::HandleObject proxy,      \
                           uint32_t begin, uint32_t end,                              \
                           js::ElementAdder* adder) {                                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->getElements(cx, proxy, begin, end, adder);                               \
 }                                                                                    \
 static bool isScriptedCb(const void* p) {                                            \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->isScripted();                                                            \
 }                                                                                    \
 static bool dynamicCheckedUnwrapAllowedCb(const void* p, JS::HandleObject obj,       \
                                           JSContext* cx) {                           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                             \
   return h->dynamicCheckedUnwrapAllowed(obj, cx);                                    \
 }                                                                                    \
 static const js::WrapperOps* ops() {                                                 \
   static const js::WrapperOps __ops = {                                              \
       {                                                                              \
           (js::ProxyFinalizeInBackgroundOp)sbx_register_cb(                          \
               (void*)finalizeInBackgroundCb, 0),                                     \
           (js::ProxyCanNurseryAllocateOp)sbx_register_cb(                            \
               (void*)canNurseryAllocateCb, 0),                                       \
           (js::ProxyEnterOp)sbx_register_cb((void*)enterCb, 0),                      \
           (js::ProxyGetOwnPropertyDescriptorOp)sbx_register_cb(                      \
               (void*)getOwnPropertyDescriptorCb, 0),                                 \
           (js::ProxyDefinePropertyOp)sbx_register_cb((void*)definePropertyCb,        \
                                                      0),                             \
           (js::ProxyOwnPropertyKeysOp)sbx_register_cb((void*)ownPropertyKeysCb,      \
                                                       0),                            \
           (js::ProxyDeleteOp)sbx_register_cb((void*)delete_Cb, 0),                   \
           (js::ProxyGetPrototypeOp)sbx_register_cb((void*)getPrototypeCb, 0),        \
           (js::ProxySetPrototypeOp)sbx_register_cb((void*)setPrototypeCb, 0),        \
           (js::ProxyGetPrototypeIfOrdinaryOp)sbx_register_cb(                        \
               (void*)getPrototypeIfOrdinaryCb, 0),                                   \
           (js::ProxySetImmutablePrototypeOp)sbx_register_cb(                         \
               (void*)setImmutablePrototypeCb, 0),                                    \
           (js::ProxyPreventExtensionsOp)sbx_register_cb(                             \
               (void*)preventExtensionsCb, 0),                                        \
           (js::ProxyIsExtensibleOp)sbx_register_cb((void*)isExtensibleCb, 0),        \
           (js::ProxyHasOp)sbx_register_cb((void*)hasCb, 0),                          \
           (js::ProxyGetOp)sbx_register_cb((void*)getCb, 0),                          \
           (js::ProxySetOp)sbx_register_cb((void*)setCb, 0),                          \
           (js::ProxyUseProxyExpandoObjectForPrivateFieldsOp)sbx_register_cb(         \
               (void*)useProxyExpandoObjectForPrivateFieldsCb, 0),                    \
           (js::ProxyThrowOnPrivateFieldOp)sbx_register_cb(                           \
               (void*)throwOnPrivateFieldCb, 0),                                      \
           (js::ProxyCallOp)sbx_register_cb((void*)callCb, 0),                        \
           (js::ProxyConstructOp)sbx_register_cb((void*)constructCb, 0),              \
           (js::ProxyEnumerateOp)sbx_register_cb((void*)enumerateCb, 0),              \
           (js::ProxyHasOwnOp)sbx_register_cb((void*)hasOwnCb, 0),                    \
           (js::ProxyGetOwnEnumerablePropertyKeysOp)sbx_register_cb(                  \
               (void*)getOwnEnumerablePropertyKeysCb, 0),                             \
           (js::ProxyNativeCallOp)sbx_register_cb((void*)nativeCallCb, 0),            \
           (js::ProxyGetBuiltinClassOp)sbx_register_cb((void*)getBuiltinClassCb,      \
                                                       0),                            \
           (js::ProxyIsArrayOp)sbx_register_cb((void*)isArrayCb, 0),                  \
           (js::ProxyClassNameOp)sbx_register_cb((void*)classNameCb, 0),              \
           (js::ProxyFunToStringOp)sbx_register_cb((void*)fun_toStringCb, 0),         \
           (js::ProxyRegexpToSharedOp)sbx_register_cb((void*)regexp_toSharedCb,       \
                                                      0),                             \
           (js::ProxyBoxedValueUnboxOp)sbx_register_cb(                               \
               (void*)boxedValue_unboxCb, 0),                                         \
           (js::ProxyTraceOp)sbx_register_cb((void*)traceCb, 0),                      \
           (js::ProxyFinalizeOp)sbx_register_cb((void*)finalizeCb, 0),                \
           (js::ProxyObjectMovedOp)sbx_register_cb((void*)objectMovedCb, 0),          \
           (js::ProxyIsCallableOp)sbx_register_cb((void*)isCallableCb, 0),            \
           (js::ProxyIsConstructorOp)sbx_register_cb((void*)isConstructorCb, 0),      \
           (js::ProxyGetElementsOp)sbx_register_cb((void*)getElementsCb, 0),          \
           (js::ProxyIsScriptedOp)sbx_register_cb((void*)isScriptedCb, 0),            \
       },                                                                             \
       (js::WrapperDynamicCheckedUnwrapAllowedOp)sbx_register_cb(                     \
           (void*)dynamicCheckedUnwrapAllowedCb, 0),                                  \
   };                                                                                 \
   return &__ops;                                                                     \
 }

class Wrapper {
private:
  js::WrapperWithOps base_;

public:
  DEFINE_WRAPPER_OPS_CALLBACKS(Wrapper)

  explicit inline Wrapper(unsigned aFlags, bool aHasPrototype = false,
                   bool aHasSecurityPolicy = false)
      : base_(ops(), this, aFlags, aHasPrototype, aHasSecurityPolicy) {}

  
  bool hasPrototype() const { return base_.hasPrototype(); }
  bool hasSecurityPolicy() const { return base_.hasSecurityPolicy(); }
  inline const void* family() const { return base_.js::BaseProxyHandler::family(); }
  unsigned flags() const { return base_.flags(); }
  bool isCrossCompartmentWrapper() const {
    return base_.isCrossCompartmentWrapper();
  }

  inline const js::WrapperWithOps* getProxyHandler() const { return &base_; }

  static JSObject* New(JSContext* cx, JSObject* obj, const Wrapper* handler,
                       const WrapperOptions& options = WrapperOptions()) {
    return js::Wrapper::New(cx, obj, handler->getProxyHandler(), options);
  }

  static JSObject* Renew(JSObject* existing, JSObject* obj,
                         const Wrapper* handler) {
    return js::Wrapper::Renew(existing, obj, handler->getProxyHandler());
  }

  static inline const Wrapper* wrapperHandler(const JSObject* wrapper) {
    return static_cast<const Wrapper*>(static_cast<const js::WrapperWithOps*>(
                                           js::Wrapper::wrapperHandler(wrapper))
                                           ->getPrivate());
  }

  static JSObject* wrappedObject(JSObject* wrapper) {
    return js::Wrapper::wrappedObject(wrapper);
  }

  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return getProxyHandler()->js::Wrapper::finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const {
    return getProxyHandler()->js::Wrapper::canNurseryAllocate();
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return getProxyHandler()->js::Wrapper::enter(cx, wrapper, id, act,
                                                          mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {
    return getProxyHandler()
        ->js::Wrapper::getOwnPropertyDescriptor(cx, proxy, id,
                                                               desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::Wrapper::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::Wrapper::ownPropertyKeys(
        cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::Wrapper::delete_(cx, proxy, id,
                                                                  result);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return getProxyHandler()->js::Wrapper::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::Wrapper::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const {
    return getProxyHandler()
        ->js::Wrapper::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return getProxyHandler()->js::Wrapper::setImmutablePrototype(
        cx, proxy, succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::Wrapper::preventExtensions(
        cx, proxy, result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const {
    return getProxyHandler()->js::Wrapper::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return getProxyHandler()->js::Wrapper::has(cx, proxy, id,
                                                              bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::Wrapper::get(cx, proxy,
                                                              receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::Wrapper::set(cx, proxy, id, v,
                                                              receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const { return false; }

  virtual bool throwOnPrivateField() const {
    return getProxyHandler()->js::Wrapper::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return getProxyHandler()->js::Wrapper::call(cx, proxy, args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return getProxyHandler()->js::Wrapper::construct(cx, proxy,
                                                                    args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::Wrapper::enumerate(cx, proxy,
                                                                    props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return getProxyHandler()->js::Wrapper::hasOwn(cx, proxy, id,
                                                                 bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->js::Wrapper::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                   props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return getProxyHandler()->js::Wrapper::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return getProxyHandler()->js::Wrapper::getBuiltinClass(
        cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return getProxyHandler()->js::Wrapper::isArray(cx, proxy,
                                                                  answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return getProxyHandler()->js::Wrapper::className(cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return getProxyHandler()->js::Wrapper::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return getProxyHandler()->js::Wrapper::regexp_toShared(
        cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::Wrapper::boxedValue_unbox(
        cx, proxy, vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return getProxyHandler()->js::Wrapper::trace(trc, proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return getProxyHandler()->js::Wrapper::finalize(gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return getProxyHandler()->js::Wrapper::objectMoved(proxy, old);
  }
  virtual bool isCallable(JSObject* obj) const {
    return getProxyHandler()->js::Wrapper::isCallable(obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return getProxyHandler()->js::Wrapper::isConstructor(obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return getProxyHandler()->js::Wrapper::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return getProxyHandler()->js::Wrapper::isScripted();
  }
  
  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,
                                           JSContext* cx) const {
    return getProxyHandler()->js::Wrapper::dynamicCheckedUnwrapAllowed(
        obj, cx);
  }
};

class CrossCompartmentWrapper {
private:
  js::CrossCompartmentWrapperWithOps base_;

public:
  DEFINE_WRAPPER_OPS_CALLBACKS(CrossCompartmentWrapper)

  explicit inline CrossCompartmentWrapper(unsigned aFlags,
                                          bool aHasPrototype = false,
                                          bool aHasSecurityPolicy = false)
      : base_(ops(), this, aFlags, aHasPrototype, aHasSecurityPolicy) {}

  operator const Wrapper*() const {
    static_assert(sizeof(CrossCompartmentWrapper) == sizeof(Wrapper));
    static_assert(sizeof(js::CrossCompartmentWrapperWithOps) == sizeof(js::WrapperWithOps));
    return reinterpret_cast<const Wrapper*>(this);
  }

  static const CrossCompartmentWrapper* getSingletonP();
      
  bool hasPrototype() const { return base_.hasPrototype(); }
  bool hasSecurityPolicy() const { return base_.hasSecurityPolicy(); }
  inline const void* family() const { return base_.js::BaseProxyHandler::family(); }
  unsigned flags() const { return base_.flags(); }
  bool isCrossCompartmentWrapper() const {
    return base_.isCrossCompartmentWrapper();
  }
  inline const js::CrossCompartmentWrapperWithOps* getProxyHandler() const {
    return &base_;
  }

  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const {
    return true;
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::enter(cx, wrapper, id, act,
                                                          mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {
    return getProxyHandler()
        ->js::CrossCompartmentWrapper::getOwnPropertyDescriptor(cx, proxy, id,
                                                               desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::ownPropertyKeys(
        cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::delete_(cx, proxy, id,
                                                                  result);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const {
    return getProxyHandler()
        ->js::CrossCompartmentWrapper::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::setImmutablePrototype(
        cx, proxy, succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::preventExtensions(
        cx, proxy, result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::has(cx, proxy, id,
                                                              bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::get(cx, proxy,
                                                              receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::set(cx, proxy, id, v,
                                                              receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const { return false; }

  virtual bool throwOnPrivateField() const {
    return getProxyHandler()->js::CrossCompartmentWrapper::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::call(cx, proxy, args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::construct(cx, proxy,
                                                                    args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::enumerate(cx, proxy,
                                                                    props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::hasOwn(cx, proxy, id,
                                                                 bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->js::CrossCompartmentWrapper::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                   props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::getBuiltinClass(
        cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::isArray(cx, proxy,
                                                                  answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::className(cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::regexp_toShared(
        cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::boxedValue_unbox(
        cx, proxy, vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::trace(trc, proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::finalize(gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::objectMoved(proxy, old);
  }
  virtual bool isCallable(JSObject* obj) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::isCallable(obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::isConstructor(obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return getProxyHandler()->js::CrossCompartmentWrapper::isScripted();
  }

  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,
                                           JSContext* cx) const {
    return getProxyHandler()->js::CrossCompartmentWrapper::dynamicCheckedUnwrapAllowed(obj, cx);
  }
};

class OpaqueCrossCompartmentWrapper {
private:
  js::OpaqueCrossCompartmentWrapperWithOps base_;
public:
  DEFINE_WRAPPER_OPS_CALLBACKS(OpaqueCrossCompartmentWrapper)
  
  explicit inline OpaqueCrossCompartmentWrapper(): base_(ops(), this) {}
  bool hasPrototype() const { return base_.hasPrototype(); }
  bool hasSecurityPolicy() const { return base_.hasSecurityPolicy(); }
  inline const void* family() const { return base_.js::BaseProxyHandler::family(); }
  unsigned flags() const { return base_.flags(); }
  bool isCrossCompartmentWrapper() const {
    return base_.isCrossCompartmentWrapper();
  }
  inline const js::OpaqueCrossCompartmentWrapperWithOps* getProxyHandler() const {
    return &base_;
  }

  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const { return true; }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::enter(cx, wrapper, id, act,
                                                          mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {
    return getProxyHandler()
        ->js::OpaqueCrossCompartmentWrapper::getOwnPropertyDescriptor(cx, proxy, id,
                                                                desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::ownPropertyKeys(
        cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::delete_(cx, proxy,
                                                                   id, result);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const {
    return getProxyHandler()
        ->js::OpaqueCrossCompartmentWrapper::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return getProxyHandler()
        ->js::OpaqueCrossCompartmentWrapper::setImmutablePrototype(cx, proxy,
                                                             succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::preventExtensions(
        cx, proxy, result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::has(cx, proxy, id,
                                                               bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::get(
        cx, proxy, receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::set(
        cx, proxy, id, v, receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const { return false; }

  virtual bool throwOnPrivateField() const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::call(cx, proxy,
                                                                args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::construct(cx, proxy,
                                                                     args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::enumerate(cx, proxy,
                                                                     props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::hasOwn(cx, proxy, id,
                                                                  bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->js::OpaqueCrossCompartmentWrapper::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                    props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::getBuiltinClass(
        cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::isArray(cx, proxy,
                                                                  answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::className(cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::regexp_toShared(
        cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::boxedValue_unbox(
        cx, proxy, vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::trace(trc, proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::finalize(gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::objectMoved(proxy, old);
  }
  virtual bool isCallable(JSObject* obj) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::isCallable(obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::isConstructor(obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::isScripted();
  }

  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,
                                           JSContext* cx) const {
    return getProxyHandler()->js::OpaqueCrossCompartmentWrapper::dynamicCheckedUnwrapAllowed(obj, cx);
  }
};

template <class Base>
class SecurityWrapper {
private:
  js::SecurityWrapperWithOps<Base> base_;
public:
  DEFINE_WRAPPER_OPS_CALLBACKS(SecurityWrapper<Base>)
 
  explicit inline SecurityWrapper(unsigned flags, bool hasPrototype = false): base_(ops(), this, flags, hasPrototype) {}

  bool hasPrototype() const { return base_.hasPrototype(); }
  bool hasSecurityPolicy() const { return base_.hasSecurityPolicy(); }
  inline const void* family() const {
    return base_.js::BaseProxyHandler::family();
  }
  unsigned flags() const { return base_.flags(); }
  bool isCrossCompartmentWrapper() const {
    return base_.isCrossCompartmentWrapper();
  }
  inline const js::SecurityWrapperWithOps<Base>* getProxyHandler() const {
    return &base_;
  }

  
  typedef Base Permissive;
  typedef js::SecurityWrapper<Base> Restrictive;

  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return getProxyHandler()
        ->Restrictive::finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const {
    return getProxyHandler()
        ->Restrictive::canNurseryAllocate();
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return getProxyHandler()->Restrictive::enter(
        cx, wrapper, id, act, mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {
    return getProxyHandler()
        ->Restrictive::getOwnPropertyDescriptor(cx, proxy,
                                                                      id, desc);
  }

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const {
    return getProxyHandler()->Restrictive::defineProperty(
        cx, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->Restrictive::ownPropertyKeys(cx, proxy, props);
  }

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const {
    return getProxyHandler()->Restrictive::delete_(
        cx, proxy, id, result);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return getProxyHandler()->Restrictive::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return getProxyHandler()->Restrictive::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const {
    return getProxyHandler()
        ->Restrictive::getPrototypeIfOrdinary(
            cx, proxy, isOrdinary, protop);
  }

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return getProxyHandler()
        ->Restrictive::setImmutablePrototype(cx, proxy,
                                                                   succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const {
    return getProxyHandler()
        ->Restrictive::preventExtensions(cx, proxy,
                                                               result);
  }

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const {
    return getProxyHandler()->Restrictive::isExtensible(
        cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return getProxyHandler()->Restrictive::has(cx, proxy,
                                                                     id, bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return getProxyHandler()->Restrictive::get(
        cx, proxy, receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return getProxyHandler()->Restrictive::set(
        cx, proxy, id, v, receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const {
    return getProxyHandler()->Restrictive::useProxyExpandoObjectForPrivateFields();
  }

  virtual bool throwOnPrivateField() const {
    return getProxyHandler()
        ->Restrictive::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return getProxyHandler()->Restrictive::call(cx, proxy,
                                                                      args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return getProxyHandler()->Restrictive::construct(
        cx, proxy, args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return getProxyHandler()->Restrictive::enumerate(
        cx, proxy, props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return getProxyHandler()->Restrictive::hasOwn(
        cx, proxy, id, bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->Restrictive::getOwnEnumerablePropertyKeys(
            cx, proxy, props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return getProxyHandler()->Restrictive::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return getProxyHandler()
        ->Restrictive::getBuiltinClass(cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return getProxyHandler()->Restrictive::isArray(
        cx, proxy, answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return getProxyHandler()->Restrictive::className(
        cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return getProxyHandler()->Restrictive::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return getProxyHandler()
        ->Restrictive::regexp_toShared(cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return getProxyHandler()
        ->Restrictive::boxedValue_unbox(cx, proxy, vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return getProxyHandler()->Restrictive::trace(trc,
                                                                       proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return getProxyHandler()->Restrictive::finalize(
        gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return getProxyHandler()->Restrictive::objectMoved(
        proxy, old);
  }
  virtual bool isCallable(JSObject* obj) const {
    return getProxyHandler()->Restrictive::isCallable(
        obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return getProxyHandler()->Restrictive::isConstructor(
        obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return getProxyHandler()->Restrictive::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return getProxyHandler()->Restrictive::isScripted();
  }

  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,
                                           JSContext* cx) const {
    return getProxyHandler()
        ->Restrictive::dynamicCheckedUnwrapAllowed(obj,
                                                                         cx);
  }
};

typedef SecurityWrapper<js::CrossCompartmentWrapper> CrossCompartmentSecurityWrapper;

inline const js::BaseProxyHandler* GetProxyHandler(const Wrapper* wrapper) {
  return wrapper->getProxyHandler();
}

inline const js::Wrapper* GetWrapper(const Wrapper* wrapper) {
  return wrapper->getProxyHandler();
}

inline const js::BaseProxyHandler* GetProxyHandler(const CrossCompartmentWrapper* wrapper) {
  return wrapper->getProxyHandler();
}

inline const js::Wrapper* GetWrapper(const CrossCompartmentWrapper* wrapper) {
  return wrapper->getProxyHandler();
}

template<class Base>
inline const js::Wrapper* GetWrapper(const SecurityWrapper<Base>* wrapper) {
  return wrapper->getProxyHandler();
}

inline const CrossCompartmentWrapper* GetCrossCompartmentWrapper(const JSObject* obj) { 
  return static_cast<const CrossCompartmentWrapper*>(static_cast<const js::CrossCompartmentWrapperWithOps*>(js::GetProxyHandler(obj))->getPrivate());
}

#else
using ForwardingProxyHandler = js::ForwardingProxyHandler;
using Wrapper = js::Wrapper;
using CrossCompartmentWrapper = js::CrossCompartmentWrapper;

inline const js::BaseProxyHandler* GetProxyHandler(const ForwardingProxyHandler* handler) {
  return handler;
}

inline const js::BaseProxyHandler* GetProxyHandler(const Wrapper* wrapper) {
  return wrapper;
}

inline const js::BaseProxyHandler* GetProxyHandler(const CrossCompartmentWrapper* wrapper) {
  return wrapper->getProxyHandler();
}

#endif
}
}
#endif
