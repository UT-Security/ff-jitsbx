/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Proxy_h
#define js_sandbox_Proxy_h

#include "js/Proxy.h"
#ifdef JS_SANDBOX_API
#include "js/sandbox/lib.h"
#include "js/Utility.h"
#endif

namespace js {
namespace sandbox {

#ifdef JS_SANDBOX_API

#define DEFINE_PROXY_HANDLER_OPS_CALLBACKS(ExternalProxyHandler)                      \
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
 static const js::ProxyHandlerOps* ops() {                                            \
   static const js::ProxyHandlerOps __ops = {                                         \
       (js::ProxyFinalizeInBackgroundOp)sbx_register_cb(                              \
           (void*)finalizeInBackgroundCb, 0),                                         \
       (js::ProxyCanNurseryAllocateOp)sbx_register_cb(                                \
           (void*)canNurseryAllocateCb, 0),                                           \
       (js::ProxyEnterOp)sbx_register_cb((void*)enterCb, 0),                          \
       (js::ProxyGetOwnPropertyDescriptorOp)sbx_register_cb(                          \
           (void*)getOwnPropertyDescriptorCb, 0),                                     \
       (js::ProxyDefinePropertyOp)sbx_register_cb((void*)definePropertyCb, 0),        \
       (js::ProxyOwnPropertyKeysOp)sbx_register_cb((void*)ownPropertyKeysCb, 0),      \
       (js::ProxyDeleteOp)sbx_register_cb((void*)delete_Cb, 0),                       \
       (js::ProxyGetPrototypeOp)sbx_register_cb((void*)getPrototypeCb, 0),            \
       (js::ProxySetPrototypeOp)sbx_register_cb((void*)setPrototypeCb, 0),            \
       (js::ProxyGetPrototypeIfOrdinaryOp)sbx_register_cb(                            \
           (void*)getPrototypeIfOrdinaryCb, 0),                                       \
       (js::ProxySetImmutablePrototypeOp)sbx_register_cb(                             \
           (void*)setImmutablePrototypeCb, 0),                                        \
       (js::ProxyPreventExtensionsOp)sbx_register_cb((void*)preventExtensionsCb,      \
                                                     0),                              \
       (js::ProxyIsExtensibleOp)sbx_register_cb((void*)isExtensibleCb, 0),            \
       (js::ProxyHasOp)sbx_register_cb((void*)hasCb, 0),                              \
       (js::ProxyGetOp)sbx_register_cb((void*)getCb, 0),                              \
       (js::ProxySetOp)sbx_register_cb((void*)setCb, 0),                              \
       (js::ProxyUseProxyExpandoObjectForPrivateFieldsOp)sbx_register_cb(             \
           (void*)useProxyExpandoObjectForPrivateFieldsCb, 0),                        \
       (js::ProxyThrowOnPrivateFieldOp)sbx_register_cb(                               \
           (void*)throwOnPrivateFieldCb, 0),                                          \
       (js::ProxyCallOp)sbx_register_cb((void*)callCb, 0),                            \
       (js::ProxyConstructOp)sbx_register_cb((void*)constructCb, 0),                  \
       (js::ProxyEnumerateOp)sbx_register_cb((void*)enumerateCb, 0),                  \
       (js::ProxyHasOwnOp)sbx_register_cb((void*)hasOwnCb, 0),                        \
       (js::ProxyGetOwnEnumerablePropertyKeysOp)sbx_register_cb(                      \
           (void*)getOwnEnumerablePropertyKeysCb, 0),                                 \
       (js::ProxyNativeCallOp)sbx_register_cb((void*)nativeCallCb, 0),                \
       (js::ProxyGetBuiltinClassOp)sbx_register_cb((void*)getBuiltinClassCb, 0),      \
       (js::ProxyIsArrayOp)sbx_register_cb((void*)isArrayCb, 0),                      \
       (js::ProxyClassNameOp)sbx_register_cb((void*)classNameCb, 0),                  \
       (js::ProxyFunToStringOp)sbx_register_cb((void*)fun_toStringCb, 0),             \
       (js::ProxyRegexpToSharedOp)sbx_register_cb((void*)regexp_toSharedCb, 0),       \
       (js::ProxyBoxedValueUnboxOp)sbx_register_cb((void*)boxedValue_unboxCb,         \
                                                   0),                                \
       (js::ProxyTraceOp)sbx_register_cb((void*)traceCb, 0),                          \
       (js::ProxyFinalizeOp)sbx_register_cb((void*)finalizeCb, 0),                    \
       (js::ProxyObjectMovedOp)sbx_register_cb((void*)objectMovedCb, 0),              \
       (js::ProxyIsCallableOp)sbx_register_cb((void*)isCallableCb, 0),                \
       (js::ProxyIsConstructorOp)sbx_register_cb((void*)isConstructorCb, 0),          \
       (js::ProxyGetElementsOp)sbx_register_cb((void*)getElementsCb, 0),              \
       (js::ProxyIsScriptedOp)sbx_register_cb((void*)isScriptedCb, 0),                \
   };                                                                                 \
   return &__ops;                                                                     \
 }

class BaseProxyHandler {
 private:
  js::BaseProxyHandler* base_;

 public:
  DEFINE_PROXY_HANDLER_OPS_CALLBACKS(BaseProxyHandler)

  explicit inline BaseProxyHandler(const void* aProxyFamily,
                                   bool aHasPrototype = false,
                                   bool aHasSecurityPolicy = false) {
    base_ = js_new<js::BaseProxyHandlerWithOps>(
        ops(), this, aProxyFamily, aHasPrototype, aHasSecurityPolicy);
  }

  explicit inline BaseProxyHandler(js::BaseProxyHandler* base)
      : base_(base) {}

  ~BaseProxyHandler() {
    js_free(reinterpret_cast<void*>(base_));
  }

  inline const js::BaseProxyHandler* getProxyHandler() const { return base_; }
  
  bool hasPrototype() const { return base_->hasPrototype(); }
  bool hasSecurityPolicy() const { return base_->hasSecurityPolicy(); }
  inline const void* family() const { return base_->family(); }

  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return getProxyHandler()->js::BaseProxyHandler::finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const {
    return getProxyHandler()->js::BaseProxyHandler::canNurseryAllocate();
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return getProxyHandler()->js::BaseProxyHandler::enter(cx, wrapper, id, act,
                                                          mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const = 0;

  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const = 0;

  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const = 0;

  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const = 0;

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return getProxyHandler()->js::BaseProxyHandler::getPrototype(cx, proxy,
                                                                 protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::BaseProxyHandler::setPrototype(cx, proxy,
                                                                 proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const = 0;

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return getProxyHandler()->js::BaseProxyHandler::setImmutablePrototype(
        cx, proxy, succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const = 0;

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const = 0;

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return getProxyHandler()->js::BaseProxyHandler::has(cx, proxy, id, bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::BaseProxyHandler::get(cx, proxy, receiver, id,
                                                        vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return getProxyHandler()->js::BaseProxyHandler::set(cx, proxy, id, v,
                                                        receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const {
    return getProxyHandler()
        ->js::BaseProxyHandler::useProxyExpandoObjectForPrivateFields();
  }

  virtual bool throwOnPrivateField() const {
    return getProxyHandler()->js::BaseProxyHandler::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return getProxyHandler()->js::BaseProxyHandler::call(cx, proxy, args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return getProxyHandler()->js::BaseProxyHandler::construct(cx, proxy, args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return getProxyHandler()->js::BaseProxyHandler::enumerate(cx, proxy, props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return getProxyHandler()->js::BaseProxyHandler::hasOwn(cx, proxy, id, bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return getProxyHandler()
        ->js::BaseProxyHandler::getOwnEnumerablePropertyKeys(cx, proxy, props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return getProxyHandler()->js::BaseProxyHandler::nativeCall(cx, test, impl,
                                                               args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return getProxyHandler()->js::BaseProxyHandler::getBuiltinClass(cx, proxy,
                                                                    cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return getProxyHandler()->js::BaseProxyHandler::isArray(cx, proxy, answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return getProxyHandler()->js::BaseProxyHandler::className(cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return getProxyHandler()->js::BaseProxyHandler::fun_toString(cx, proxy,
                                                                 isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return getProxyHandler()->js::BaseProxyHandler::regexp_toShared(cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return getProxyHandler()->js::BaseProxyHandler::boxedValue_unbox(cx, proxy,
                                                                     vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return getProxyHandler()->js::BaseProxyHandler::trace(trc, proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return getProxyHandler()->js::BaseProxyHandler::finalize(gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return getProxyHandler()->js::BaseProxyHandler::objectMoved(proxy, old);
  }

  virtual bool isCallable(JSObject* obj) const {
    return getProxyHandler()->js::BaseProxyHandler::isCallable(obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return getProxyHandler()->js::BaseProxyHandler::isConstructor(obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return getProxyHandler()->js::BaseProxyHandler::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return getProxyHandler()->js::BaseProxyHandler::isScripted();
  }
};

inline const BaseProxyHandler* GetProxyHandler(const JSObject* obj) {
  return static_cast<const BaseProxyHandler*>(static_cast<const js::BaseProxyHandlerWithOps*>(js::GetProxyHandler(obj))->getPrivate());
}

inline const js::BaseProxyHandler* GetProxyHandler(const BaseProxyHandler* handler) {
  return handler->getProxyHandler();
}

#else
using BaseProxyHandler = js::BaseProxyHandler;
using GetProxyHandler = js::GetProxyHandler;
#endif
 
}
}

#endif
