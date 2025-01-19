/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Proxy_h
#define js_sandbox_Proxy_h

#include "js/Proxy.h"
#ifdef JS_SANDBOX_API
#include "js/sandbox/sobox.h"
#endif

namespace js {
namespace sandbox {

#ifdef JS_SANDBOX_API

class BaseProxyHandler {
 private:
  js::BaseProxyHandlerWithOps base_;

 public:
  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return base_.js::BaseProxyHandler::finalizeInBackground(priv);
  }
  
  static bool finalizeInBackgroundCb(const void* p, const JS::Value& priv) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->finalizeInBackground(priv);
  }

  virtual bool canNurseryAllocate() const {
    return base_.js::BaseProxyHandler::canNurseryAllocate();
  }
  static bool canNurseryAllocateCb(const void* p) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->canNurseryAllocate();
  }
  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return base_.js::BaseProxyHandler::enter(cx, wrapper, id, act, mayThrow,
                                             bp);
  }
  static bool enterCb(const void* p, JSContext* cx, JS::HandleObject wrapper,
                    JS::HandleId id, js::BaseProxyHandler::Action act,
                    bool mayThrow, bool* bp) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->enter(cx, wrapper, id, act, mayThrow, bp);
  }

  virtual bool getOwnPropertyDescriptor(
      JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const = 0;
  static bool getOwnPropertyDescriptorCb(
      const void* p, JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->getOwnPropertyDescriptor(cx, proxy, id, desc);
  }
  virtual bool defineProperty(JSContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const = 0;
  static bool definePropertyCb(const void* p, JSContext* cx,
                             JS::HandleObject proxy, JS::HandleId id,
                             JS::Handle<JS::PropertyDescriptor> desc,
                             JS::ObjectOpResult& result) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->defineProperty(cx, proxy, id, desc, result);
  }
  virtual bool ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const = 0;
  static bool ownPropertyKeysCb(const void* p, JSContext* cx,
                              JS::HandleObject proxy,
                              JS::MutableHandleIdVector props) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->ownPropertyKeys(cx, proxy, props);
  }
  virtual bool delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const = 0;
  static bool delete_Cb(const void* p, JSContext* cx, JS::HandleObject proxy,
                      JS::HandleId id, JS::ObjectOpResult& result) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->delete_(cx, proxy, id, result);
  }

  virtual bool getPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return base_.js::BaseProxyHandler::getPrototype(cx, proxy, protop);
  }
  static bool getPrototypeCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                           JS::MutableHandleObject protop) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->getPrototype(cx, proxy, protop);
  }
  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return base_.js::BaseProxyHandler::setPrototype(cx, proxy, proto, result);
  }
  static bool setPrototypeCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                           JS::HandleObject proto, JS::ObjectOpResult& result) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->setPrototype(cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const = 0;
  static bool getPrototypeIfOrdinaryCb(const void* p, JSContext* cx,
                                     JS::HandleObject proxy, bool* isOrdinary,
                                     JS::MutableHandleObject protop) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->getPrototypeIfOrdinary(cx, proxy, isOrdinary, protop);
  }
  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return base_.js::BaseProxyHandler::setImmutablePrototype(cx, proxy,
                                                             succeeded);
  }
  static bool setImmutablePrototypeCb(const void* p, JSContext* cx,
                                    JS::HandleObject proxy, bool* succeeded) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->setImmutablePrototype(cx, proxy, succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const = 0;
  static bool preventExtensionsCb(const void* p, JSContext* cx,
                                JS::HandleObject proxy,
                                JS::ObjectOpResult& result) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->preventExtensions(cx, proxy, result);
  }
  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const = 0;
  static bool isExtensibleCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                           bool* extensible) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->isExtensible(cx, proxy, extensible);
  }

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return base_.js::BaseProxyHandler::has(cx, proxy, id, bp);
  }
  static bool hasCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                  JS::HandleId id, bool* bp) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->has(cx, proxy, id, bp);
  }
  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return base_.js::BaseProxyHandler::get(cx, proxy, receiver, id, vp);
  }
  static bool getCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                  JS::HandleValue receiver, JS::HandleId id,
                  JS::MutableHandleValue vp) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->get(cx, proxy, receiver, id, vp);
  }
  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return base_.js::BaseProxyHandler::set(cx, proxy, id, v, receiver, result);
  }
  static bool setCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                  JS::HandleId id, JS::HandleValue v, JS::HandleValue receiver,
                  JS::ObjectOpResult& result) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->set(cx, proxy, id, v, receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const {
    return base_.js::BaseProxyHandler::useProxyExpandoObjectForPrivateFields();
  }
  static bool useProxyExpandoObjectForPrivateFieldsCb(const void* p) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->useProxyExpandoObjectForPrivateFields();
  }

  virtual bool throwOnPrivateField() const {
    return base_.js::BaseProxyHandler::throwOnPrivateField();
  }
  static bool throwOnPrivateFieldCb(const void* p) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return base_.js::BaseProxyHandler::call(cx, proxy, args);
  }
  static bool callCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                   const JS::CallArgs& args) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->call(cx, proxy, args);
  }
  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return base_.js::BaseProxyHandler::construct(cx, proxy, args);
  }
  static bool constructCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                        const JS::CallArgs& args) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->construct(cx, proxy, args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return base_.js::BaseProxyHandler::enumerate(cx, proxy, props);
  }
  static bool enumerateCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                        JS::MutableHandleIdVector props) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->enumerate(cx, proxy, props);
  }
  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return base_.js::BaseProxyHandler::hasOwn(cx, proxy, id, bp);
  }
  static bool hasOwnCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                     JS::HandleId id, bool* bp) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->hasOwn(cx, proxy, id, bp);
  }
  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return base_.js::BaseProxyHandler::getOwnEnumerablePropertyKeys(cx, proxy,
                                                                    props);
  }
  static bool getOwnEnumerablePropertyKeysCb(const void* p, JSContext* cx,
                                           JS::HandleObject proxy,
                                           JS::MutableHandleIdVector props) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->getOwnEnumerablePropertyKeys(cx, proxy, props);
  }
  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return base_.js::BaseProxyHandler::nativeCall(cx, test, impl, args);
  }
  static bool nativeCallCb(const void* p, JSContext* cx,
                         JS::IsAcceptableThis test, JS::NativeImpl impl,
                         const JS::CallArgs& args) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->nativeCall(cx, test, impl, args);
  }
  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return base_.js::BaseProxyHandler::getBuiltinClass(cx, proxy, cls);
  }
  static bool getBuiltinClassCb(const void* p, JSContext* cx,
                              JS::HandleObject proxy, js::ESClass* cls) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->getBuiltinClass(cx, proxy, cls);
  }
  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return base_.js::BaseProxyHandler::isArray(cx, proxy, answer);
  }
  static bool isArrayCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                      JS::IsArrayAnswer* answer) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->isArray(cx, proxy, answer);
  }
  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return base_.js::BaseProxyHandler::className(cx, proxy);
  }
  static const char* classNameCb(const void* p, JSContext* cx,
                               JS::HandleObject proxy) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->className(cx, proxy);
  }
  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return base_.js::BaseProxyHandler::fun_toString(cx, proxy, isToSource);
  }
  static JSString* fun_toStringCb(const void* p, JSContext* cx,
                                JS::HandleObject proxy, bool isToSource) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->fun_toString(cx, proxy, isToSource);
  }
  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return base_.js::BaseProxyHandler::regexp_toShared(cx, proxy);
  }
  static js::RegExpShared* regexp_toSharedCb(const void* p, JSContext* cx,
                                           JS::HandleObject proxy) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->regexp_toShared(cx, proxy);
  }
  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return base_.js::BaseProxyHandler::boxedValue_unbox(cx, proxy, vp);
  }
  static bool boxedValue_unboxCb(const void* p, JSContext* cx,
                               JS::HandleObject proxy,
                               JS::MutableHandleValue vp) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->boxedValue_unbox(cx, proxy, vp);
  }
  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return base_.js::BaseProxyHandler::trace(trc, proxy);
  }
  static void traceCb(const void* p, JSTracer* trc, JSObject* proxy) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->trace(trc, proxy);
  }
  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return base_.js::BaseProxyHandler::finalize(gcx, proxy);
  }
  static void finalizeCb(const void* p, JS::GCContext* gcx, JSObject* proxy) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->finalize(gcx, proxy);
  }
  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return base_.js::BaseProxyHandler::objectMoved(proxy, old);
  }
  static size_t objectMovedCb(const void* p, JSObject* proxy, JSObject* old) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->objectMoved(proxy, old);
  }

  virtual bool isCallable(JSObject* obj) const {
    return base_.js::BaseProxyHandler::isCallable(obj);
  }
  static bool isCallableCb(const void* p, JSObject* obj) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->isCallable(obj);
  }
  virtual bool isConstructor(JSObject* obj) const {
    return base_.js::BaseProxyHandler::isConstructor(obj);
  }
  static bool isConstructorCb(const void* p, JSObject* obj) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->isConstructor(obj);
  }
  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return base_.js::BaseProxyHandler::getElements(cx, proxy, begin, end,
                                                   adder);
  }
  static bool getElementsCb(const void* p, JSContext* cx, JS::HandleObject proxy,
                          uint32_t begin, uint32_t end,
                          js::ElementAdder* adder) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->getElements(cx, proxy, begin, end, adder);
  }
  virtual bool isScripted() const {
    return base_.js::BaseProxyHandler::isScripted();
  }
  static bool isScriptedCb(const void* p) {
    auto* h = static_cast<const BaseProxyHandler*>(p);
    return h->isScripted();
  }

  bool hasPrototype() const { return base_.hasPrototype(); }

  bool hasSecurityPolicy() const { return base_.hasSecurityPolicy(); }

  inline const void* family() const { return base_.family(); }

  static const js::ProxyHandlerOps* ops() {
    static const js::ProxyHandlerOps __ops = {
        (js::ProxyFinalizeInBackgroundOp)sbx_register_cb(
            (void*)finalizeInBackgroundCb, 0),
        (js::ProxyCanNurseryAllocateOp)sbx_register_cb(
            (void*)canNurseryAllocateCb, 0),
        (js::ProxyEnterOp)sbx_register_cb((void*)enterCb, 0),
        (js::ProxyGetOwnPropertyDescriptorOp)sbx_register_cb(
            (void*)getOwnPropertyDescriptorCb, 0),
        (js::ProxyDefinePropertyOp)sbx_register_cb(
            (void*)definePropertyCb, 0),
        (js::ProxyOwnPropertyKeysOp)sbx_register_cb(
            (void*)ownPropertyKeysCb, 0),
        (js::ProxyDeleteOp)sbx_register_cb((void*)delete_Cb, 0),
        (js::ProxyGetPrototypeOp)sbx_register_cb((void*)getPrototypeCb,
                                                          0),
        (js::ProxySetPrototypeOp)sbx_register_cb((void*)setPrototypeCb,
                                                          0),
        (js::ProxyGetPrototypeIfOrdinaryOp)sbx_register_cb(
            (void*)getPrototypeIfOrdinaryCb, 0),
        (js::ProxySetImmutablePrototypeOp)sbx_register_cb(
            (void*)setImmutablePrototypeCb, 0),
        (js::ProxyPreventExtensionsOp)sbx_register_cb(
            (void*)preventExtensionsCb, 0),
        (js::ProxyIsExtensibleOp)sbx_register_cb((void*)isExtensibleCb,
                                                          0),
        (js::ProxyHasOp)sbx_register_cb((void*)hasCb, 0),
        (js::ProxyGetOp)sbx_register_cb((void*)getCb, 0),
        (js::ProxySetOp)sbx_register_cb((void*)setCb, 0),
        (js::ProxyUseProxyExpandoObjectForPrivateFieldsOp)
            sbx_register_cb((void*)useProxyExpandoObjectForPrivateFieldsCb, 0),
        (js::ProxyThrowOnPrivateFieldOp)sbx_register_cb(
            (void*)throwOnPrivateFieldCb, 0),
        (js::ProxyCallOp)sbx_register_cb((void*)callCb, 0),
        (js::ProxyConstructOp)sbx_register_cb((void*)constructCb, 0),
        (js::ProxyEnumerateOp)sbx_register_cb((void*)enumerateCb, 0),
        (js::ProxyHasOwnOp)sbx_register_cb((void*)hasOwnCb, 0),
        (js::ProxyGetOwnEnumerablePropertyKeysOp)sbx_register_cb(
            (void*)getOwnEnumerablePropertyKeysCb, 0),
        (js::ProxyNativeCallOp)sbx_register_cb((void*)nativeCallCb, 0),
        (js::ProxyGetBuiltinClassOp)sbx_register_cb(
            (void*)getBuiltinClassCb, 0),
        (js::ProxyIsArrayOp)sbx_register_cb((void*)isArrayCb, 0),
        (js::ProxyClassNameOp)sbx_register_cb((void*)classNameCb, 0),
        (js::ProxyFunToStringOp)sbx_register_cb((void*)fun_toStringCb,
                                                         0),
        (js::ProxyRegexpToSharedOp)sbx_register_cb(
            (void*)regexp_toSharedCb, 0),
        (js::ProxyBoxedValueUnboxOp)sbx_register_cb(
            (void*)boxedValue_unboxCb, 0),
        (js::ProxyTraceOp)sbx_register_cb((void*)traceCb, 0),
        (js::ProxyFinalizeOp)sbx_register_cb((void*)finalizeCb, 0),
        (js::ProxyObjectMovedOp)sbx_register_cb((void*)objectMovedCb,
                                                         0),
        (js::ProxyIsCallableOp)sbx_register_cb((void*)isCallableCb, 0),
        (js::ProxyIsConstructorOp)sbx_register_cb(
            (void*)isConstructorCb, 0),
        (js::ProxyGetElementsOp)sbx_register_cb((void*)getElementsCb,
                                                         0),
        (js::ProxyIsScriptedOp)sbx_register_cb((void*)isScriptedCb, 0),
    };

    return &__ops;
  }

  explicit inline BaseProxyHandler(const void* aProxyFamily,
                                         bool aHasPrototype = false)
      : base_(ops(), this, aProxyFamily, aHasPrototype) {}

  inline const js::BaseProxyHandler* getProxyHandler() const { return &base_; }
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
