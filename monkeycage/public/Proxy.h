/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Proxy_h
#define mc_Proxy_h

#ifdef JS_SANDBOX
#include "js/sandbox/Proxy.h"

namespace mc {

#define DEFINE_PROXY_HANDLER_OPS_CALLBACKS(ExternalProxyHandler)                             \
 static bool finalizeInBackgroundCb(const void* p, const JS::Value& priv) {                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->finalizeInBackground(priv);                                                     \
 }                                                                                           \
 static bool canNurseryAllocateCb(const void* p) {                                           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->canNurseryAllocate();                                                           \
 }                                                                                           \
 static bool enterCb(const void* p, JSContext* cx, JS::HandleObject wrapper,                 \
                     JS::HandleId id, js::BaseProxyHandler::Action act,                      \
                     bool mayThrow, bool* bp) {                                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->enter(cx, wrapper, id, act, mayThrow, bp);                                      \
 }                                                                                           \
 static bool getOwnPropertyDescriptorCb(                                                     \
     const void* p, JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                  \
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->getOwnPropertyDescriptor(cx, proxy, id, desc);                                  \
 }                                                                                           \
 static bool definePropertyCb(const void* p, JSContext* cx,                                  \
                              JS::HandleObject proxy, JS::HandleId id,                       \
                              JS::Handle<JS::PropertyDescriptor> desc,                       \
                              JS::ObjectOpResult& result) {                                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->defineProperty(cx, proxy, id, desc, result);                                    \
 }                                                                                           \
 static bool ownPropertyKeysCb(const void* p, JSContext* cx,                                 \
                               JS::HandleObject proxy,                                       \
                               JS::MutableHandleIdVector props) {                            \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->ownPropertyKeys(cx, proxy, props);                                              \
 }                                                                                           \
 static bool delete_Cb(const void* p, JSContext* cx, JS::HandleObject proxy,                 \
                       JS::HandleId id, JS::ObjectOpResult& result) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->delete_(cx, proxy, id, result);                                                 \
 }                                                                                           \
 static bool getPrototypeCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy,                                          \
                            JS::MutableHandleObject protop) {                                \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->getPrototype(cx, proxy, protop);                                                \
 }                                                                                           \
 static bool setPrototypeCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy, JS::HandleObject proto,                  \
                            JS::ObjectOpResult& result) {                                    \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->setPrototype(cx, proxy, proto, result);                                         \
 }                                                                                           \
 static bool getPrototypeIfOrdinaryCb(const void* p, JSContext* cx,                          \
                                      JS::HandleObject proxy, bool* isOrdinary,              \
                                      JS::MutableHandleObject protop) {                      \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->getPrototypeIfOrdinary(cx, proxy, isOrdinary, protop);                          \
 }                                                                                           \
 static bool setImmutablePrototypeCb(const void* p, JSContext* cx,                           \
                                     JS::HandleObject proxy, bool* succeeded) {              \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->setImmutablePrototype(cx, proxy, succeeded);                                    \
 }                                                                                           \
 static bool preventExtensionsCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy,                                     \
                                 JS::ObjectOpResult& result) {                               \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->preventExtensions(cx, proxy, result);                                           \
 }                                                                                           \
 static bool isExtensibleCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy, bool* extensible) {                      \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->isExtensible(cx, proxy, extensible);                                            \
 }                                                                                           \
 static bool hasCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleId id, bool* bp) {                                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->has(cx, proxy, id, bp);                                                         \
 }                                                                                           \
 static bool getCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleValue receiver, JS::HandleId id,                                \
                   JS::MutableHandleValue vp) {                                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->get(cx, proxy, receiver, id, vp);                                               \
 }                                                                                           \
 static bool setCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleId id, JS::HandleValue v, JS::HandleValue receiver,             \
                   JS::ObjectOpResult& result) {                                             \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->set(cx, proxy, id, v, receiver, result);                                        \
 }                                                                                           \
 static bool useProxyExpandoObjectForPrivateFieldsCb(const void* p) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->useProxyExpandoObjectForPrivateFields();                                        \
 }                                                                                           \
 static bool throwOnPrivateFieldCb(const void* p) {                                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->throwOnPrivateField();                                                          \
 }                                                                                           \
 static bool callCb(const void* p, JSContext* cx, JS::HandleObject proxy,                    \
                    const JS::CallArgs& args) {                                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->call(cx, proxy, args);                                                          \
 }                                                                                           \
 static bool constructCb(const void* p, JSContext* cx, JS::HandleObject proxy,               \
                         const JS::CallArgs& args) {                                         \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->construct(cx, proxy, args);                                                     \
 }                                                                                           \
 static bool enumerateCb(const void* p, JSContext* cx, JS::HandleObject proxy,               \
                         JS::MutableHandleIdVector props) {                                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->enumerate(cx, proxy, props);                                                    \
 }                                                                                           \
 static bool hasOwnCb(const void* p, JSContext* cx, JS::HandleObject proxy,                  \
                      JS::HandleId id, bool* bp) {                                           \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->hasOwn(cx, proxy, id, bp);                                                      \
 }                                                                                           \
 static bool getOwnEnumerablePropertyKeysCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy,                          \
                                            JS::MutableHandleIdVector props) {               \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->getOwnEnumerablePropertyKeys(cx, proxy, props);                                 \
 }                                                                                           \
 static bool nativeCallCb(const void* p, JSContext* cx,                                      \
                          JS::IsAcceptableThis test, JS::NativeImpl impl,                    \
                          const JS::CallArgs& args) {                                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->nativeCall(cx, test, impl, args);                                               \
 }                                                                                           \
 static bool getBuiltinClassCb(const void* p, JSContext* cx,                                 \
                               JS::HandleObject proxy, js::ESClass* cls) {                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->getBuiltinClass(cx, proxy, cls);                                                \
 }                                                                                           \
 static bool isArrayCb(const void* p, JSContext* cx, JS::HandleObject proxy,                 \
                       JS::IsArrayAnswer* answer) {                                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->isArray(cx, proxy, answer);                                                     \
 }                                                                                           \
 static const char* classNameCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy) {                                    \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->className(cx, proxy);                                                           \
 }                                                                                           \
 static JSString* fun_toStringCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy, bool isToSource) {                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->fun_toString(cx, proxy, isToSource);                                            \
 }                                                                                           \
 static js::RegExpShared* regexp_toSharedCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->regexp_toShared(cx, proxy);                                                     \
 }                                                                                           \
 static bool boxedValue_unboxCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy,                                      \
                                JS::MutableHandleValue vp) {                                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->boxedValue_unbox(cx, proxy, vp);                                                \
 }                                                                                           \
 static void traceCb(const void* p, JSTracer* trc, JSObject* proxy) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->trace(trc, proxy);                                                              \
 }                                                                                           \
 static void finalizeCb(const void* p, JS::GCContext* gcx, JSObject* proxy) {                \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->finalize(gcx, proxy);                                                           \
 }                                                                                           \
 static size_t objectMovedCb(const void* p, JSObject* proxy, JSObject* old) {                \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->objectMoved(proxy, old);                                                        \
 }                                                                                           \
 static bool isCallableCb(const void* p, JSObject* obj) {                                    \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->isCallable(obj);                                                                \
 }                                                                                           \
 static bool isConstructorCb(const void* p, JSObject* obj) {                                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->isConstructor(obj);                                                             \
 }                                                                                           \
 static bool getElementsCb(const void* p, JSContext* cx, JS::HandleObject proxy,             \
                           uint32_t begin, uint32_t end,                                     \
                           js::ElementAdder* adder) {                                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->getElements(cx, proxy, begin, end, adder);                                      \
 }                                                                                           \
 static bool isScriptedCb(const void* p) {                                                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   return h->isScripted();                                                                   \
 }                                                                                           \
 static const js::sandbox::ProxyHandlerOps* ops() {                                          \
   static const js::sandbox::ProxyHandlerOps __ops = {                                       \
       .finalizeInBackground = finalizeInBackgroundCb,                                       \
       .canNurseryAllocate = canNurseryAllocateCb,                                           \
       .enter = enterCb,                                                                     \
       .getOwnPropertyDescriptor = getOwnPropertyDescriptorCb,                               \
       .defineProperty = definePropertyCb,                                                   \
       .ownPropertyKeys = ownPropertyKeysCb,                                                 \
       .delete_ = delete_Cb,                                                                 \
       .getPrototype = getPrototypeCb,                                                       \
       .setPrototype = setPrototypeCb,                                                       \
       .getPrototypeIfOrdinary = getPrototypeIfOrdinaryCb,                                   \
       .setImmutablePrototype = setImmutablePrototypeCb,                                     \
       .preventExtensions = preventExtensionsCb,                                             \
       .isExtensible = isExtensibleCb,                                                       \
       .has = hasCb,                                                                         \
       .get = getCb,                                                                         \
       .set = setCb,                                                                         \
       .useProxyExpandoObjectForPrivateFields = useProxyExpandoObjectForPrivateFieldsCb,     \
       .throwOnPrivateField = throwOnPrivateFieldCb,                                         \
       .call = callCb,                                                                       \
       .construct = constructCb,                                                             \
       .enumerate = enumerateCb,                                                             \
       .hasOwn = hasOwnCb,                                                                   \
       .getOwnEnumerablePropertyKeys = getOwnEnumerablePropertyKeysCb,                       \
       .nativeCall = nativeCallCb,                                                           \
       .getBuiltinClass = getBuiltinClassCb,                                                 \
       .isArray = isArrayCb,                                                                 \
       .className = classNameCb,                                                             \
       .fun_toString = fun_toStringCb,                                                       \
       .regexp_toShared = regexp_toSharedCb,                                                 \
       .boxedValue_unbox = boxedValue_unboxCb,                                               \
       .trace = traceCb,                                                                     \
       .finalize = finalizeCb,                                                               \
       .objectMoved = objectMovedCb,                                                         \
       .isCallable = isCallableCb,                                                           \
       .isConstructor = isConstructorCb,                                                     \
       .getElements = getElementsCb,                                                         \
       .isScripted = isScriptedCb,                                                           \
   };                                                                                        \
   return &__ops;                                                                            \
 }

class BaseProxyHandler {
  const js::BaseProxyHandler* inner_;

 DEFINE_PROXY_HANDLER_OPS_CALLBACKS(BaseProxyHandler)

 public:
  explicit BaseProxyHandler(const void* aFamily, bool aHasPrototype = false,
                            bool aHasSecurityPolicy = false) {
    inner_ = js_new<js::sandbox::BaseProxyHandler>(
        ops(), this, aFamily, aHasPrototype, aHasSecurityPolicy);
  }

  ~BaseProxyHandler() { js_free((void*)inner_); }
  
  bool hasPrototype() const { return UNSAFE_getProxyHandler()->hasPrototype(); }

  bool hasSecurityPolicy() const { return UNSAFE_getProxyHandler()->hasSecurityPolicy(); }

  inline const void* family() const { return UNSAFE_getProxyHandler()->family(); }

  inline const js::BaseProxyHandler* UNSAFE_getProxyHandler() const { return inner_; }
  
  virtual bool finalizeInBackground(const JS::Value& priv) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::finalizeInBackground(
        priv);
  }

  virtual bool canNurseryAllocate() const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::canNurseryAllocate();
  }

  virtual bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     js::BaseProxyHandler::Action act, bool mayThrow,
                     bool* bp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::enter(
        cx, wrapper, id, act, mayThrow, bp);
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
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::getPrototype(
        cx, proxy, protop);
  }

  virtual bool setPrototype(JSContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::setPrototype(
        cx, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject protop) const = 0;

  virtual bool setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::setImmutablePrototype(cx, proxy, succeeded);
  }

  virtual bool preventExtensions(JSContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const = 0;

  virtual bool isExtensible(JSContext* cx, JS::HandleObject proxy,
                            bool* extensible) const = 0;

  virtual bool has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::has(cx, proxy, id,
                                                               bp);
  }

  virtual bool get(JSContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::get(
        cx, proxy, receiver, id, vp);
  }

  virtual bool set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::set(
        cx, proxy, id, v, receiver, result);
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::useProxyExpandoObjectForPrivateFields();
  }

  virtual bool throwOnPrivateField() const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::throwOnPrivateField();
  }

  virtual bool call(JSContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::call(cx, proxy,
                                                                args);
  }

  virtual bool construct(JSContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::construct(cx, proxy,
                                                                     args);
  }

  virtual bool enumerate(JSContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::enumerate(cx, proxy,
                                                                     props);
  }

  virtual bool hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::hasOwn(cx, proxy, id,
                                                                  bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      JSContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::getOwnEnumerablePropertyKeys(cx, proxy, props);
  }

  virtual bool nativeCall(JSContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::nativeCall(
        cx, test, impl, args);
  }

  virtual bool getBuiltinClass(JSContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::getBuiltinClass(
        cx, proxy, cls);
  }

  virtual bool isArray(JSContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::isArray(cx, proxy,
                                                                   answer);
  }

  virtual const char* className(JSContext* cx, JS::HandleObject proxy) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::className(cx, proxy);
  }

  virtual JSString* fun_toString(JSContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::fun_toString(
        cx, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(JSContext* cx,
                                            JS::HandleObject proxy) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::regexp_toShared(
        cx, proxy);
  }

  virtual bool boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::boxedValue_unbox(
        cx, proxy, vp);
  }

  virtual void trace(JSTracer* trc, JSObject* proxy) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::trace(trc, proxy);
  }

  virtual void finalize(JS::GCContext* gcx, JSObject* proxy) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::finalize(gcx, proxy);
  }

  virtual size_t objectMoved(JSObject* proxy, JSObject* old) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::objectMoved(proxy,
                                                                       old);
  }

  virtual bool isCallable(JSObject* obj) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::isCallable(obj);
  }

  virtual bool isConstructor(JSObject* obj) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::isConstructor(obj);
  }

  virtual bool getElements(JSContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           js::ElementAdder* adder) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::getElements(
        cx, proxy, begin, end, adder);
  }

  virtual bool isScripted() const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::isScripted();
  }
};

// TODO(abhishek): move to js namespace once argument is Tainted
inline const BaseProxyHandler* GetProxyHandler(const JSObject* obj) {
  // TODO(abhishek): both casts are unsafe and need to be tainted - probably by maintaining a table of
  // valid monkeycage::BaseProxyHandler pointers.
  return static_cast<const BaseProxyHandler*>(
      static_cast<const js::sandbox::BaseProxyHandler*>(js::GetProxyHandler(obj))
          ->getHandler());
}

}  // namespace mc

namespace js {

inline JSObject* NewProxyObject(JSContext* cx,
                                const mc::BaseProxyHandler* handler,
                                JS::HandleValue priv, JSObject* proto,
                                const ProxyOptions& options = ProxyOptions()) {
  return NewProxyObject(cx, handler->UNSAFE_getProxyHandler(), priv, proto,
                        options);
}

}  // namespace js
#else
#include "js/Proxy.h"

namespace mc {
using BaseProxyHandler = js::BaseProxyHandler;

// TODO(abhishek): remove once argument is Tainted
inline const BaseProxyHandler* GetProxyHandler(const JSObject* obj) {
  return js::GetProxyHandler(obj);
}

}
#endif




#ifdef JS_SANDBOX

namespace mc {
extern const JSClassOps* ProxyClassOps();
extern const js::ClassExtension* ProxyClassExtension();
extern const js::ObjectOps* ProxyObjectOps();
}

#define MONKEYCAGE_PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, classSpec) \
  {                                                                        \
    name,                                                                  \
        JSClass::NON_NATIVE | JSCLASS_IS_PROXY |                           \
            JSCLASS_DELAY_METADATA_BUILDER | js::CheckProxyFlags<flags>(), \
        mc::ProxyClassOps(), classSpec, mc::ProxyClassExtension(),         \
        mc::ProxyObjectOps()                                               \
  }

#define MONKEYCAGE_PROXY_CLASS_DEF(name, flags) \
  MONKEYCAGE_PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, JS_NULL_CLASS_SPEC)

#else

#define MONKEYCAGE_PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, classSpec) \
  PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, classSpec)

#define MONKEYCAGE_PROXY_CLASS_DEF(name, flags) \
  PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, JS_NULL_CLASS_SPEC)

#endif

#endif
