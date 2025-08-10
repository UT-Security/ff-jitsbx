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
      MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getOwnPropertyDescriptor(cx->cx_, proxy, id,
                                                               desc);
  }

  virtual bool defineProperty(MCContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::defineProperty(
        cx->cx_, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(MCContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::ownPropertyKeys(cx->cx_, proxy, props);
  }

  virtual bool delete_(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::delete_(
        cx->cx_, proxy, id, result);
  }

  virtual bool enumerate(MCContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::enumerate(
        cx->cx_, proxy, props);
  }

  virtual bool getPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::getPrototype(
        cx->cx_, proxy, protop);
  }

  virtual bool setPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::setPrototype(
        cx->cx_, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(
      MCContext* cx, JS::HandleObject proxy, MC::Tainted<bool*> isOrdinary,
      JS::MutableHandleObject protop) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getPrototypeIfOrdinary(
            cx->cx_, proxy, isOrdinary.UNSAFE_unverified(), protop);
  }

  virtual bool setImmutablePrototype(MCContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::setImmutablePrototype(cx->cx_, proxy,
                                                            succeeded);
  }

  virtual bool preventExtensions(MCContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::preventExtensions(cx->cx_, proxy, result);
  }

  virtual bool isExtensible(MCContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::isExtensible(
        cx->cx_, proxy, extensible);
  }

  virtual bool has(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::has(cx->cx_, proxy,
                                                                     id, bp);
  }

  virtual bool get(MCContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::get(
        cx->cx_, proxy, receiver, id, vp);
  }

  virtual bool set(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::set(
        cx->cx_, proxy, id, v, receiver, result);
  }

  virtual bool call(MCContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::call(cx->cx_, proxy,
                                                                      args);
  }

  virtual bool construct(MCContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::construct(
        cx->cx_, proxy, args);
  }

  virtual bool hasOwn(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::hasOwn(
        cx->cx_, proxy, id, bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      MCContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getOwnEnumerablePropertyKeys(cx->cx_, proxy,
                                                                   props);
  }

  virtual bool nativeCall(MCContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::nativeCall(
        cx->cx_, test, impl, args);
  }

  virtual bool getBuiltinClass(MCContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::getBuiltinClass(cx->cx_, proxy, cls);
  }

  virtual bool isArray(MCContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::isArray(
        cx->cx_, proxy, answer);
  }

  virtual const char* className(MCContext* cx,
                                JS::HandleObject proxy) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::className(
        cx->cx_, proxy);
  }

  virtual JSString* fun_toString(MCContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const override {
    return UNSAFE_getProxyHandler()->js::ForwardingProxyHandler::fun_toString(
        cx->cx_, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(
      MCContext* cx, JS::HandleObject proxy) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::regexp_toShared(cx->cx_, proxy);
  }

  virtual bool boxedValue_unbox(MCContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override {
    return UNSAFE_getProxyHandler()
        ->js::ForwardingProxyHandler::boxedValue_unbox(cx->cx_, proxy, vp);
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
                     bool mayThrow, bool* bp_) {                                             \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   MC::Tainted<bool*> bp{nullptr};                                                           \
   bp.assign_raw_pointer(bp_);                                                               \
   return h->enter(mcx, wrapper, id, act, mayThrow, bp);                                     \
 }                                                                                           \
 static bool getOwnPropertyDescriptorCb(                                                     \
     const void* p, JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                  \
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {                       \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->getOwnPropertyDescriptor(mcx, proxy, id, desc);                                 \
 }                                                                                           \
 static bool definePropertyCb(const void* p, JSContext* cx,                                  \
                              JS::HandleObject proxy, JS::HandleId id,                       \
                              JS::Handle<JS::PropertyDescriptor> desc,                       \
                              JS::ObjectOpResult& result) {                                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->defineProperty(mcx, proxy, id, desc, result);                                   \
 }                                                                                           \
 static bool ownPropertyKeysCb(const void* p, JSContext* cx,                                 \
                               JS::HandleObject proxy,                                       \
                               JS::MutableHandleIdVector props) {                            \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->ownPropertyKeys(mcx, proxy, props);                                             \
 }                                                                                           \
 static bool delete_Cb(const void* p, JSContext* cx, JS::HandleObject proxy,                 \
                       JS::HandleId id, JS::ObjectOpResult& result) {                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->delete_(mcx, proxy, id, result);                                                \
 }                                                                                           \
 static bool getPrototypeCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy,                                          \
                            JS::MutableHandleObject protop) {                                \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->getPrototype(mcx, proxy, protop);                                               \
 }                                                                                           \
 static bool setPrototypeCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy, JS::HandleObject proto,                  \
                            JS::ObjectOpResult& result) {                                    \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->setPrototype(mcx, proxy, proto, result);                                        \
 }                                                                                           \
 static bool getPrototypeIfOrdinaryCb(const void* p, JSContext* cx,                          \
                                      JS::HandleObject proxy, bool* isOrdinary_,             \
                                      JS::MutableHandleObject protop) {                      \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   MC::Tainted<bool*> isOrdinary{nullptr};                                                   \
   isOrdinary.assign_raw_pointer(isOrdinary_);                                               \
   return h->getPrototypeIfOrdinary(mcx, proxy, isOrdinary, protop);                         \
 }                                                                                           \
 static bool setImmutablePrototypeCb(const void* p, JSContext* cx,                           \
                                     JS::HandleObject proxy, bool* succeeded) {              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->setImmutablePrototype(mcx, proxy, succeeded);                                   \
 }                                                                                           \
 static bool preventExtensionsCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy,                                     \
                                 JS::ObjectOpResult& result) {                               \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->preventExtensions(mcx, proxy, result);                                          \
 }                                                                                           \
 static bool isExtensibleCb(const void* p, JSContext* cx,                                    \
                            JS::HandleObject proxy, bool* extensible) {                      \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->isExtensible(mcx, proxy, extensible);                                           \
 }                                                                                           \
 static bool hasCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleId id, bool* bp) {                                              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->has(mcx, proxy, id, bp);                                                        \
 }                                                                                           \
 static bool getCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleValue receiver, JS::HandleId id,                                \
                   JS::MutableHandleValue vp) {                                              \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->get(mcx, proxy, receiver, id, vp);                                              \
 }                                                                                           \
 static bool setCb(const void* p, JSContext* cx, JS::HandleObject proxy,                     \
                   JS::HandleId id, JS::HandleValue v, JS::HandleValue receiver,             \
                   JS::ObjectOpResult& result) {                                             \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->set(mcx, proxy, id, v, receiver, result);                                       \
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
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->call(mcx, proxy, args);                                                         \
 }                                                                                           \
 static bool constructCb(const void* p, JSContext* cx, JS::HandleObject proxy,               \
                         const JS::CallArgs& args) {                                         \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->construct(mcx, proxy, args);                                                    \
 }                                                                                           \
 static bool enumerateCb(const void* p, JSContext* cx, JS::HandleObject proxy,               \
                         JS::MutableHandleIdVector props) {                                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->enumerate(mcx, proxy, props);                                                   \
 }                                                                                           \
 static bool hasOwnCb(const void* p, JSContext* cx, JS::HandleObject proxy,                  \
                      JS::HandleId id, bool* bp) {                                           \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->hasOwn(mcx, proxy, id, bp);                                                     \
 }                                                                                           \
 static bool getOwnEnumerablePropertyKeysCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy,                          \
                                            JS::MutableHandleIdVector props) {               \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->getOwnEnumerablePropertyKeys(mcx, proxy, props);                                \
 }                                                                                           \
 static bool nativeCallCb(const void* p, JSContext* cx,                                      \
                          JS::IsAcceptableThis test, JS::NativeImpl impl,                    \
                          const JS::CallArgs& args) {                                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->nativeCall(mcx, test, impl, args);                                              \
 }                                                                                           \
 static bool getBuiltinClassCb(const void* p, JSContext* cx,                                 \
                               JS::HandleObject proxy, js::ESClass* cls) {                   \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->getBuiltinClass(mcx, proxy, cls);                                               \
 }                                                                                           \
 static bool isArrayCb(const void* p, JSContext* cx, JS::HandleObject proxy,                 \
                       JS::IsArrayAnswer* answer) {                                          \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->isArray(mcx, proxy, answer);                                                    \
 }                                                                                           \
 static const char* classNameCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy) {                                    \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->className(mcx, proxy);                                                          \
 }                                                                                           \
 static JSString* fun_toStringCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy, bool isToSource) {                  \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->fun_toString(mcx, proxy, isToSource);                                           \
 }                                                                                           \
 static js::RegExpShared* regexp_toSharedCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy) {                        \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->regexp_toShared(mcx, proxy);                                                    \
 }                                                                                           \
 static bool boxedValue_unboxCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy,                                      \
                                JS::MutableHandleValue vp) {                                 \
   auto* h = static_cast<const ExternalWrapper*>(p);                                         \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->boxedValue_unbox(mcx, proxy, vp);                                               \
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
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->getElements(mcx, proxy, begin, end, adder);                                     \
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
     MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
     const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::getOwnPropertyDescriptor(cx->cx_, proxy, id,
                                                               desc);
 }

 virtual bool defineProperty(MCContext* cx, JS::HandleObject proxy,
                             JS::HandleId id,
                             JS::Handle<JS::PropertyDescriptor> desc,
                             JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::defineProperty(
       cx->cx_, proxy, id, desc, result);
 }

 virtual bool ownPropertyKeys(MCContext* cx, JS::HandleObject proxy,
                              JS::MutableHandleIdVector props) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::ownPropertyKeys(
       cx->cx_, proxy, props);
 }

 virtual bool delete_(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::delete_(cx->cx_, proxy,
                                                                    id, result);
 }

 virtual bool enumerate(MCContext* cx, JS::HandleObject proxy,
                        JS::MutableHandleIdVector props) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::enumerate(cx->cx_, proxy,
                                                                      props);
 }

 virtual bool getPrototype(MCContext* cx, JS::HandleObject proxy,
                           JS::MutableHandleObject protop) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::getPrototype(
       cx->cx_, proxy, protop);
 }

 virtual bool setPrototype(MCContext* cx, JS::HandleObject proxy,
                           JS::HandleObject proto,
                           JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::setPrototype(
       cx->cx_, proxy, proto, result);
 }

 virtual bool getPrototypeIfOrdinary(
     MCContext* cx, JS::HandleObject proxy, MC::Tainted<bool*> isOrdinary,
     JS::MutableHandleObject protop) const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::getPrototypeIfOrdinary(
           cx->cx_, proxy, isOrdinary.UNSAFE_unverified(), protop);
 }

 virtual bool setImmutablePrototype(MCContext* cx, JS::HandleObject proxy,
                                    bool* succeeded) const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::setImmutablePrototype(cx->cx_, proxy,
                                                            succeeded);
 }

 virtual bool preventExtensions(MCContext* cx, JS::HandleObject proxy,
                                JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::preventExtensions(
       cx->cx_, proxy, result);
 }

 virtual bool isExtensible(MCContext* cx, JS::HandleObject proxy,
                           bool* extensible) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::isExtensible(
       cx->cx_, proxy, extensible);
 }

 virtual bool has(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                  bool* bp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::has(cx->cx_, proxy, id,
                                                                bp);
 }

 virtual bool get(MCContext* cx, JS::HandleObject proxy,
                  JS::HandleValue receiver, JS::HandleId id,
                  JS::MutableHandleValue vp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::get(
       cx->cx_, proxy, receiver, id, vp);
 }

 virtual bool set(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                  JS::HandleValue v, JS::HandleValue receiver,
                  JS::ObjectOpResult& result) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::set(
       cx->cx_, proxy, id, v, receiver, result);
 }

 virtual bool call(MCContext* cx, JS::HandleObject proxy,
                   const JS::CallArgs& args) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::call(cx->cx_, proxy,
                                                                 args);
 }

 virtual bool construct(MCContext* cx, JS::HandleObject proxy,
                        const JS::CallArgs& args) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::construct(cx->cx_, proxy,
                                                                      args);
 }

 virtual bool hasOwn(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                     bool* bp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::hasOwn(cx->cx_, proxy,
                                                                   id, bp);
 }

 virtual bool getOwnEnumerablePropertyKeys(
     MCContext* cx, JS::HandleObject proxy,
     JS::MutableHandleIdVector props) const override {
   return UNSAFE_getWrapper()
       ->js::CrossCompartmentWrapper::getOwnEnumerablePropertyKeys(cx->cx_, proxy,
                                                                   props);
 }

 virtual bool nativeCall(MCContext* cx, JS::IsAcceptableThis test,
                         JS::NativeImpl impl,
                         const JS::CallArgs& args) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::nativeCall(
       cx->cx_, test, impl, args);
 }

 virtual const char* className(MCContext* cx,
                               JS::HandleObject proxy) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::className(cx->cx_,
                                                                      proxy);
 }

 virtual JSString* fun_toString(MCContext* cx, JS::HandleObject proxy,
                                bool isToSource) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::fun_toString(
       cx->cx_, proxy, isToSource);
 }

 virtual js::RegExpShared* regexp_toShared(
     MCContext* cx, JS::HandleObject proxy) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::regexp_toShared(
       cx->cx_, proxy);
 }

 virtual bool boxedValue_unbox(MCContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleValue vp) const override {
   return UNSAFE_getWrapper()->js::CrossCompartmentWrapper::boxedValue_unbox(
       cx->cx_, proxy, vp);
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
      MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getOwnPropertyDescriptor(cx->cx_, proxy, id,
                                                                desc);
  }

  virtual bool defineProperty(MCContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::defineProperty(
        cx->cx_, proxy, id, desc, result);
  }

  virtual bool ownPropertyKeys(MCContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::ownPropertyKeys(
        cx->cx_, proxy, props);
  }

  virtual bool delete_(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::delete_(
        cx->cx_, proxy, id, result);
  }

  virtual bool enumerate(MCContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::enumerate(
        cx->cx_, proxy, props);
  }

  virtual bool getPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::getPrototype(
        cx->cx_, proxy, protop);
  }

  virtual bool setPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::setPrototype(
        cx->cx_, proxy, proto, result);
  }

  virtual bool getPrototypeIfOrdinary(
      MCContext* cx, JS::HandleObject proxy, MC::Tainted<bool*> isOrdinary,
      JS::MutableHandleObject protop) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getPrototypeIfOrdinary(
            cx->cx_, proxy, isOrdinary.UNSAFE_unverified(), protop);
  }

  virtual bool setImmutablePrototype(MCContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::setImmutablePrototype(cx->cx_, proxy,
                                                             succeeded);
  }

  virtual bool preventExtensions(MCContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::preventExtensions(
        cx->cx_, proxy, result);
  }

  virtual bool isExtensible(MCContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::isExtensible(
        cx->cx_, proxy, extensible);
  }

  virtual bool has(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   bool* bp) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::has(cx->cx_, proxy, id,
                                                                 bp);
  }

  virtual bool get(MCContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::get(
        cx->cx_, proxy, receiver, id, vp);
  }

  virtual bool set(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::set(
        cx->cx_, proxy, id, v, receiver, result);
  }

  virtual bool call(MCContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::call(cx->cx_, proxy,
                                                                  args);
  }

  virtual bool construct(MCContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::construct(
        cx->cx_, proxy, args);
  }

  virtual bool hasOwn(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      bool* bp) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::hasOwn(cx->cx_, proxy,
                                                                    id, bp);
  }

  virtual bool getOwnEnumerablePropertyKeys(
      MCContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getOwnEnumerablePropertyKeys(cx->cx_, proxy,
                                                                    props);
  }

  virtual bool getBuiltinClass(MCContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const override {
    return UNSAFE_getWrapper()
        ->js::OpaqueCrossCompartmentWrapper::getBuiltinClass(cx->cx_, proxy, cls);
  }

  virtual bool isArray(MCContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::isArray(
        cx->cx_, proxy, answer);
  }

  virtual const char* className(MCContext* cx, JS::HandleObject proxy) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::className(
        cx->cx_, proxy);
  }

  virtual JSString* fun_toString(MCContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const override {
    return UNSAFE_getWrapper()->js::OpaqueCrossCompartmentWrapper::fun_toString(
        cx->cx_, proxy, isToSource);
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

  virtual bool enter(MCContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     BaseProxyHandler::Action act, bool mayThrow,
                     MC::Tainted<bool*> bp) const override {
    return UNSAFE_getWrapper()->Unsafe::enter(cx->cx_, wrapper, id, act, mayThrow, bp.UNSAFE_unverified());
  }

  virtual bool defineProperty(MCContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->Unsafe::defineProperty(cx->cx_, proxy, id, desc,
                                                     result);
  }

  virtual bool isExtensible(MCContext* cx, JS::HandleObject proxy,
                            bool* extensible) const override {
    return UNSAFE_getWrapper()->Unsafe::isExtensible(cx->cx_, proxy, extensible);
  }

  virtual bool preventExtensions(MCContext* cx, JS::HandleObject proxy,
                                 JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->Unsafe::preventExtensions(cx->cx_, proxy, result);
  }

  virtual bool setPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            JS::ObjectOpResult& result) const override {
    return UNSAFE_getWrapper()->Unsafe::setPrototype(cx->cx_, proxy, proto, result);
  }

  virtual bool setImmutablePrototype(MCContext* cx, JS::HandleObject proxy,
                                     bool* succeeded) const override {
    return UNSAFE_getWrapper()->Unsafe::setImmutablePrototype(cx->cx_, proxy,
                                                            succeeded);
  }

  virtual bool nativeCall(MCContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const override {
    return UNSAFE_getWrapper()->Unsafe::nativeCall(cx->cx_, test, impl, args);
  }

  virtual bool getBuiltinClass(MCContext* cx, JS::HandleObject proxy,
                               js::ESClass* cls) const override {
    return UNSAFE_getWrapper()->Unsafe::getBuiltinClass(cx->cx_, proxy, cls);
  }

  virtual bool isArray(MCContext* cx, JS::HandleObject proxy,
                       JS::IsArrayAnswer* answer) const override {
    return UNSAFE_getWrapper()->Unsafe::isArray(cx->cx_, proxy, answer);
  }

  virtual js::RegExpShared* regexp_toShared(MCContext* cx,
                                            JS::HandleObject proxy) const override {
    return UNSAFE_getWrapper()->Unsafe::regexp_toShared(cx->cx_, proxy);
  }

  virtual bool boxedValue_unbox(MCContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const override {
    return UNSAFE_getWrapper()->Unsafe::boxedValue_unbox(cx->cx_, proxy, vp);
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

//TODO(abhishek): readd default for stopAtWindowProxy when call is unambiguous
inline JSObject* UncheckedUnwrap(JSObject* obj,
                                        bool stopAtWindowProxy,
                                        MC::Tainted<unsigned*> flagsp) {
    return UncheckedUnwrap(obj, stopAtWindowProxy, flagsp.UNSAFE_unverified());
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
