/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Proxy_h
#define mc_Proxy_h

#ifdef JS_SANDBOX
#include "js/sandbox/Proxy.h"

#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"
#include "monkeycage/Tainted.h"

#include "mozilla/Assertions.h"

#include <shared_mutex>
#include <unordered_set>

namespace mc {

#define DEFINE_PROXY_HANDLER_OPS_CALLBACKS(ExternalProxyHandler)                             \
 static MC::Tainted<bool> finalizeInBackgroundCb(MC::AppPointer<const void*> p, const JS::Value& priv) {  \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                \
   return h->finalizeInBackground(priv);                                                     \
 }                                                                                           \
 static MC::Tainted<bool> canNurseryAllocateCb(MC::AppPointer<const void*> p) {                                           \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   return h->canNurseryAllocate();                                                           \
 }                                                                                           \
 static MC::Tainted<bool> enterCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject wrapper,                 \
                     JS::HandleId id, BaseProxyHandler::Action act,                          \
                     bool mayThrow, MC::Tainted<bool*> bp) {                                             \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->enter(cx, wrapper, id, act, mayThrow, bp);                                     \
 }                                                                                           \
 static MC::Tainted<bool> getOwnPropertyDescriptorCb(                                                     \
     MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy, JS::HandleId id,                  \
     JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) {                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->getOwnPropertyDescriptor(cx, proxy, id, desc);                                 \
 }                                                                                           \
 static MC::Tainted<bool> definePropertyCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                  \
                              JS::HandleObject proxy, JS::HandleId id,                       \
                              JS::Handle<JS::PropertyDescriptor> desc,                       \
                              JS::ObjectOpResult& result_) {                                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   MC::Tainted<JS::ObjectOpResult*> result{nullptr};                                         \
   result.assign_raw_pointer(&result_);                                                      \
   return h->defineProperty(cx, proxy, id, desc, result);                                   \
 }                                                                                           \
 static MC::Tainted<bool> ownPropertyKeysCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                 \
                               JS::HandleObject proxy,                                       \
                               JS::MutableHandleIdVector props) {                            \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->ownPropertyKeys(cx, proxy, props);                                             \
 }                                                                                           \
 static MC::Tainted<bool> delete_Cb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                 \
                       JS::HandleId id, JS::ObjectOpResult& result_) {                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   MC::Tainted<JS::ObjectOpResult*> result{nullptr};                                         \
   result.assign_raw_pointer(&result_);                                                      \
   return h->delete_(cx, proxy, id, result);                                                \
 }                                                                                           \
 static MC::Tainted<bool> getPrototypeCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                    \
                            JS::HandleObject proxy,                                          \
                            JS::MutableHandleObject protop) {                                \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->getPrototype(cx, proxy, protop);                                               \
 }                                                                                           \
 static MC::Tainted<bool> setPrototypeCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                    \
                            JS::HandleObject proxy, JS::HandleObject proto,                  \
                            JS::ObjectOpResult& result_) {                                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   MC::Tainted<JS::ObjectOpResult*> result{nullptr};                                         \
   result.assign_raw_pointer(&result_);                                                      \
   return h->setPrototype(cx, proxy, proto, result);                                        \
 }                                                                                           \
 static MC::Tainted<bool> getPrototypeIfOrdinaryCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                          \
                                      JS::HandleObject proxy, MC::Tainted<bool*> isOrdinary,             \
                                      JS::MutableHandleObject protop) {                      \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->getPrototypeIfOrdinary(cx, proxy, isOrdinary, protop);                         \
 }                                                                                           \
 static MC::Tainted<bool> setImmutablePrototypeCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                           \
                                     JS::HandleObject proxy, MC::Tainted<bool*> succeeded) {             \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->setImmutablePrototype(cx, proxy, succeeded);                                   \
 }                                                                                           \
 static MC::Tainted<bool> preventExtensionsCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                               \
                                 JS::HandleObject proxy,                                     \
                                 JS::ObjectOpResult& result_) {                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   MC::Tainted<JS::ObjectOpResult*> result{nullptr};                                         \
   result.assign_raw_pointer(&result_);                                                      \
   return h->preventExtensions(cx, proxy, result);                                          \
 }                                                                                           \
 static MC::Tainted<bool> isExtensibleCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                    \
                            JS::HandleObject proxy, MC::Tainted<bool*> extensible) {                     \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->isExtensible(cx, proxy, extensible);                                           \
 }                                                                                           \
 static MC::Tainted<bool> hasCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                     \
                   JS::HandleId id, MC::Tainted<bool*> bp) {                                             \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->has(cx, proxy, id, bp);                                                        \
 }                                                                                           \
 static MC::Tainted<bool> getCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                     \
                   JS::HandleValue receiver, JS::HandleId id,                                \
                   JS::MutableHandleValue vp) {                                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->get(cx, proxy, receiver, id, vp);                                              \
 }                                                                                           \
 static MC::Tainted<bool> setCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                     \
                   JS::HandleId id, JS::HandleValue v, JS::HandleValue receiver,             \
                   JS::ObjectOpResult& result_) {                                            \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   MC::Tainted<JS::ObjectOpResult*> result{nullptr};                                         \
   result.assign_raw_pointer(&result_);                                                      \
   return h->set(cx, proxy, id, v, receiver, result);                                       \
 }                                                                                           \
 static MC::Tainted<bool> useProxyExpandoObjectForPrivateFieldsCb(MC::AppPointer<const void*> p) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   return h->useProxyExpandoObjectForPrivateFields();                                        \
 }                                                                                           \
 static MC::Tainted<bool> throwOnPrivateFieldCb(MC::AppPointer<const void*> p) {                                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   return h->throwOnPrivateField();                                                          \
 }                                                                                           \
 static MC::Tainted<bool> callCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                    \
                    const JS::CallArgs& args) {                                              \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->call(cx, proxy, args);                                                          \
 }                                                                                           \
 static MC::Tainted<bool> constructCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,               \
                         const JS::CallArgs& args) {                                         \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->construct(cx, proxy, args);                                                    \
 }                                                                                           \
 static MC::Tainted<bool> enumerateCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,               \
                         JS::MutableHandleIdVector props) {                                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->enumerate(cx, proxy, props);                                                   \
 }                                                                                           \
 static MC::Tainted<bool> hasOwnCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                  \
                      JS::HandleId id, MC::Tainted<bool*> bp) {                                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->hasOwn(cx, proxy, id, bp);                                                     \
 }                                                                                           \
 static MC::Tainted<bool> getOwnEnumerablePropertyKeysCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                    \
                                            JS::HandleObject proxy,                          \
                                            JS::MutableHandleIdVector props) {               \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->getOwnEnumerablePropertyKeys(cx, proxy, props);                                \
 }                                                                                           \
 static bool nativeCallCb(const void* p, JSContext* cx,                                      \
                          JS::IsAcceptableThis test, JS::NativeImpl impl,                    \
                          const JS::CallArgs& args) {                                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->nativeCall(mcx, test, impl, args);                                              \
 }                                                                                           \
 static MC::Tainted<bool> getBuiltinClassCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                 \
                               JS::HandleObject proxy, MC::Tainted<js::ESClass*> cls) {                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->getBuiltinClass(cx, proxy, cls);                                               \
 }                                                                                           \
 static MC::Tainted<bool> isArrayCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,                 \
                       MC::Tainted<JS::IsArrayAnswer*> answer) {                                          \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->isArray(cx, proxy, answer);                                                    \
 }                                                                                           \
 static const char* classNameCb(const void* p, JSContext* cx,                                \
                                JS::HandleObject proxy) {                                    \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->className(mcx, proxy);                                                          \
 }                                                                                           \
 static JSString* fun_toStringCb(const void* p, JSContext* cx,                               \
                                 JS::HandleObject proxy, bool isToSource) {                  \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->fun_toString(mcx, proxy, isToSource);                                           \
 }                                                                                           \
 static js::RegExpShared* regexp_toSharedCb(const void* p, JSContext* cx,                    \
                                            JS::HandleObject proxy) {                        \
   auto* h = static_cast<const ExternalProxyHandler*>(p);                                    \
   MCContext* mcx = JS_SanitizeContext(cx);                                                  \
   return h->regexp_toShared(mcx, proxy);                                                    \
 }                                                                                           \
 static MC::Tainted<bool> boxedValue_unboxCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx,                                \
                                JS::HandleObject proxy,                                      \
                                JS::MutableHandleValue vp) {                                 \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->boxedValue_unbox(cx, proxy, vp);                                               \
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
 static MC::Tainted<bool> getElementsCb(MC::AppPointer<const void*> p, MC::Tainted<JSContext*> tcx, JS::HandleObject proxy,             \
                           uint32_t begin, uint32_t end,                                     \
                           MC::Tainted<js::ElementAdder*> adder) {                                       \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   MCContext* cx = tcx.copy_and_verify_address(MC_VerifyContext);                                                  \
   return h->getElements(cx, proxy, begin, end, adder);                                     \
 }                                                                                           \
 static MC::Tainted<bool> isScriptedCb(MC::AppPointer<const void*> p) {                                                   \
   auto* h = static_cast<const ExternalProxyHandler*>(p.UNSAFE_unverified());                                    \
   return h->isScripted();                                                                   \
 }                                                                                           \
 static const js::sandbox::ProxyHandlerOps* ops() {                                          \
   static const js::sandbox::ProxyHandlerOps __ops = {                                       \
       .finalizeInBackground = MC::Sandbox::RegisterTaintedCallback(finalizeInBackgroundCb).UNSAFE_get(),                                       \
       .canNurseryAllocate = MC::Sandbox::RegisterTaintedCallback(canNurseryAllocateCb).UNSAFE_get(),                                           \
       .enter = MC::Sandbox::RegisterTaintedCallback(enterCb).UNSAFE_get(),                                                                     \
       .getOwnPropertyDescriptor = MC::Sandbox::RegisterTaintedCallback(getOwnPropertyDescriptorCb).UNSAFE_get(),                               \
       .defineProperty = MC::Sandbox::RegisterTaintedCallback(definePropertyCb).UNSAFE_get(),                                                   \
       .ownPropertyKeys = MC::Sandbox::RegisterTaintedCallback(ownPropertyKeysCb).UNSAFE_get(),                                                 \
       .delete_ = MC::Sandbox::RegisterTaintedCallback(delete_Cb).UNSAFE_get(),                                                                 \
       .getPrototype = MC::Sandbox::RegisterTaintedCallback(getPrototypeCb).UNSAFE_get(),                                                       \
       .setPrototype = MC::Sandbox::RegisterTaintedCallback(setPrototypeCb).UNSAFE_get(),                                                       \
       .getPrototypeIfOrdinary = MC::Sandbox::RegisterTaintedCallback(getPrototypeIfOrdinaryCb).UNSAFE_get(),                                   \
       .setImmutablePrototype = MC::Sandbox::RegisterTaintedCallback(setImmutablePrototypeCb).UNSAFE_get(),                                     \
       .preventExtensions = MC::Sandbox::RegisterTaintedCallback(preventExtensionsCb).UNSAFE_get(),                                             \
       .isExtensible = MC::Sandbox::RegisterTaintedCallback(isExtensibleCb).UNSAFE_get(),                                                       \
       .has = MC::Sandbox::RegisterTaintedCallback(hasCb).UNSAFE_get(),                                                                         \
       .get = MC::Sandbox::RegisterTaintedCallback(getCb).UNSAFE_get(),                                                                         \
       .set = MC::Sandbox::RegisterTaintedCallback(setCb).UNSAFE_get(),                                                                         \
       .useProxyExpandoObjectForPrivateFields = MC::Sandbox::RegisterTaintedCallback(useProxyExpandoObjectForPrivateFieldsCb).UNSAFE_get(),     \
       .throwOnPrivateField = MC::Sandbox::RegisterTaintedCallback(throwOnPrivateFieldCb).UNSAFE_get(),                                         \
       .call = MC::Sandbox::RegisterTaintedCallback(callCb).UNSAFE_get(),                                                                       \
       .construct = MC::Sandbox::RegisterTaintedCallback(constructCb).UNSAFE_get(),                                                             \
       .enumerate = MC::Sandbox::RegisterTaintedCallback(enumerateCb).UNSAFE_get(),                                                             \
       .hasOwn = MC::Sandbox::RegisterTaintedCallback(hasOwnCb).UNSAFE_get(),                                                                   \
       .getOwnEnumerablePropertyKeys = MC::Sandbox::RegisterTaintedCallback(getOwnEnumerablePropertyKeysCb).UNSAFE_get(),                       \
       .nativeCall = MC::Sandbox::RegisterCallback(nativeCallCb).UNSAFE_get(),                                                           \
       .getBuiltinClass = MC::Sandbox::RegisterTaintedCallback(getBuiltinClassCb).UNSAFE_get(),                                                 \
       .isArray = MC::Sandbox::RegisterTaintedCallback(isArrayCb).UNSAFE_get(),                                                                 \
       .className = MC::Sandbox::RegisterCallback(classNameCb).UNSAFE_get(),                                                             \
       .fun_toString = MC::Sandbox::RegisterCallback(fun_toStringCb).UNSAFE_get(),                                                       \
       .regexp_toShared = MC::Sandbox::RegisterCallback(regexp_toSharedCb).UNSAFE_get(),                                                 \
       .boxedValue_unbox = MC::Sandbox::RegisterTaintedCallback(boxedValue_unboxCb).UNSAFE_get(),                                               \
       .trace = MC::Sandbox::RegisterCallback(traceCb).UNSAFE_get(),                                                                     \
       .finalize = MC::Sandbox::RegisterCallback(finalizeCb).UNSAFE_get(),                                                               \
       .objectMoved = MC::Sandbox::RegisterCallback(objectMovedCb).UNSAFE_get(),                                                         \
       .isCallable = MC::Sandbox::RegisterCallback(isCallableCb).UNSAFE_get(),                                                           \
       .isConstructor = MC::Sandbox::RegisterCallback(isConstructorCb).UNSAFE_get(),                                                     \
       .getElements = MC::Sandbox::RegisterTaintedCallback(getElementsCb).UNSAFE_get(),                                                         \
       .isScripted = MC::Sandbox::RegisterTaintedCallback(isScriptedCb).UNSAFE_get(),                                                           \
   };                                                                                                                                    \
   return &__ops;                                                                                                                        \
 }

class BaseProxyHandler {
  const js::BaseProxyHandler* inner_;
  bool owned_;

  static inline std::unordered_set<const void*> ptr_table;
  static inline std::shared_mutex ptr_table_mutex;
public:
  static bool isValid(const void* ptr) {
    std::shared_lock lock(ptr_table_mutex);
    return ptr_table.find(ptr) != ptr_table.end(); 
  }
  using Action = js::BaseProxyHandler::Action;
private:
 DEFINE_PROXY_HANDLER_OPS_CALLBACKS(BaseProxyHandler)

 public:
  explicit BaseProxyHandler(const void* aFamily, bool aHasPrototype = false,
                            bool aHasSecurityPolicy = false) {
    inner_ = js_new<js::sandbox::BaseProxyHandler>(
        ops(), this, aFamily, aHasPrototype, aHasSecurityPolicy);
    owned_ = true;
    std::unique_lock lock(ptr_table_mutex);
    ptr_table.insert(this);
  }

  explicit inline BaseProxyHandler(const js::BaseProxyHandler* inner, bool owned = true)
      : inner_(inner), owned_(owned) {
    if (owned_) {
      std::unique_lock lock(ptr_table_mutex);
      ptr_table.insert(this);
    }
  }

  ~BaseProxyHandler() {
    if (owned_) {
      {
        std::unique_lock lock(ptr_table_mutex);
        ptr_table.erase(this);
      }
      js_free((void*)inner_);
    }
  }

  
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


  virtual bool enter(MCContext* cx, JS::HandleObject wrapper, JS::HandleId id,
                     Action act, bool mayThrow, MC::Tainted<bool*> bp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::enter(
        cx->cx_, wrapper, id, act, mayThrow, bp.UNSAFE_unverified());
  }

  virtual bool getOwnPropertyDescriptor(
      MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const = 0;

  virtual bool defineProperty(MCContext* cx, JS::HandleObject proxy,
                              JS::HandleId id,
                              JS::Handle<JS::PropertyDescriptor> desc,
                              MC::Tainted<JS::ObjectOpResult*> result) const = 0;

  virtual bool ownPropertyKeys(MCContext* cx, JS::HandleObject proxy,
                               JS::MutableHandleIdVector props) const = 0;

  virtual bool delete_(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                       MC::Tainted<JS::ObjectOpResult*> result) const = 0;

  virtual bool getPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::MutableHandleObject protop) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::getPrototype(
        cx->cx_, proxy, protop);
  }

  virtual bool setPrototype(MCContext* cx, JS::HandleObject proxy,
                            JS::HandleObject proto,
                            MC::Tainted<JS::ObjectOpResult*> result) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::setPrototype(
        cx->cx_, proxy, proto, *result.INTERNAL_unverified_safe());
  }

  virtual bool getPrototypeIfOrdinary(MCContext* cx, JS::HandleObject proxy,
                                      MC::Tainted<bool*> isOrdinary,
                                      JS::MutableHandleObject protop) const = 0;

  virtual bool setImmutablePrototype(MCContext* cx, JS::HandleObject proxy,
                                     MC::Tainted<bool*> succeeded) const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::setImmutablePrototype(
            cx->cx_, proxy, succeeded.INTERNAL_unverified_safe());
  }

  virtual bool preventExtensions(MCContext* cx, JS::HandleObject proxy,
                                 MC::Tainted<JS::ObjectOpResult*> result) const = 0;

  virtual bool isExtensible(MCContext* cx, JS::HandleObject proxy,
                            MC::Tainted<bool*> extensible) const = 0;

  virtual bool has(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   MC::Tainted<bool*> bp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::has(
        cx->cx_, proxy, id, bp.INTERNAL_unverified_safe());
  }

  virtual bool get(MCContext* cx, JS::HandleObject proxy,
                   JS::HandleValue receiver, JS::HandleId id,
                   JS::MutableHandleValue vp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::get(
        cx->cx_, proxy, receiver, id, vp);
  }

  virtual bool set(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                   JS::HandleValue v, JS::HandleValue receiver,
                   MC::Tainted<JS::ObjectOpResult*> result) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::set(
        cx->cx_, proxy, id, v, receiver, *result.INTERNAL_unverified_safe());
  }

  virtual bool useProxyExpandoObjectForPrivateFields() const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::useProxyExpandoObjectForPrivateFields();
  }

  virtual bool throwOnPrivateField() const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::throwOnPrivateField();
  }

  virtual bool call(MCContext* cx, JS::HandleObject proxy,
                    const JS::CallArgs& args) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::call(cx->cx_, proxy,
                                                                args);
  }

  virtual bool construct(MCContext* cx, JS::HandleObject proxy,
                         const JS::CallArgs& args) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::construct(cx->cx_, proxy,
                                                                     args);
  }

  virtual bool enumerate(MCContext* cx, JS::HandleObject proxy,
                         JS::MutableHandleIdVector props) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::enumerate(cx->cx_, proxy,
                                                                     props);
  }

  virtual bool hasOwn(MCContext* cx, JS::HandleObject proxy, JS::HandleId id,
                      MC::Tainted<bool*> bp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::hasOwn(
        cx->cx_, proxy, id, bp.INTERNAL_unverified_safe());
  }

  virtual bool getOwnEnumerablePropertyKeys(
      MCContext* cx, JS::HandleObject proxy,
      JS::MutableHandleIdVector props) const {
    return UNSAFE_getProxyHandler()
        ->js::BaseProxyHandler::getOwnEnumerablePropertyKeys(cx->cx_, proxy, props);
  }

  virtual bool nativeCall(MCContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl, const JS::CallArgs& args) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::nativeCall(
        cx->cx_, test, impl, args);
  }

  virtual bool getBuiltinClass(MCContext* cx, JS::HandleObject proxy,
                               MC::Tainted<js::ESClass*> cls) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::getBuiltinClass(
        cx->cx_, proxy, cls.INTERNAL_unverified_safe());
  }

  virtual bool isArray(MCContext* cx, JS::HandleObject proxy,
                       MC::Tainted<JS::IsArrayAnswer*> answer) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::isArray(cx->cx_, proxy,
                                                                   answer.INTERNAL_unverified_safe());
  }

  virtual const char* className(MCContext* cx, JS::HandleObject proxy) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::className(cx->cx_, proxy);
  }

  virtual JSString* fun_toString(MCContext* cx, JS::HandleObject proxy,
                                 bool isToSource) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::fun_toString(
        cx->cx_, proxy, isToSource);
  }

  virtual js::RegExpShared* regexp_toShared(MCContext* cx,
                                            JS::HandleObject proxy) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::regexp_toShared(
        cx->cx_, proxy);
  }

  virtual bool boxedValue_unbox(MCContext* cx, JS::HandleObject proxy,
                                JS::MutableHandleValue vp) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::boxedValue_unbox(
        cx->cx_, proxy, vp);
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

  virtual bool getElements(MCContext* cx, JS::HandleObject proxy,
                           uint32_t begin, uint32_t end,
                           MC::Tainted<js::ElementAdder*> adder) const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::getElements(
        cx->cx_, proxy, begin, end, adder.INTERNAL_unverified_safe());
  }

  virtual bool isScripted() const {
    return UNSAFE_getProxyHandler()->js::BaseProxyHandler::isScripted();
  }
};

inline bool IsProxyHandler(const JSObject* obj, const BaseProxyHandler* handler) {
  return js::GetProxyHandler(obj) == handler->UNSAFE_getProxyHandler();
}

// TODO(abhishek): move to js namespace once argument is Tainted
inline const BaseProxyHandler* GetProxyHandler(const JSObject* obj) {
  const void* ptr =
      static_cast<const js::sandbox::BaseProxyHandler*>(
          js::GetProxyHandler(obj))
          ->getHandler();

  MOZ_RELEASE_ASSERT(BaseProxyHandler::isValid(ptr), "Unexpeted mc::BaseProxyHandler app pointer");
  return static_cast<const BaseProxyHandler*>(ptr);
}

inline const void* GetProxyHandlerFamily(const JSObject* obj) {
  return js::GetProxyHandler(obj)->family();
}

inline bool IsScriptedProxy(const JSObject* obj) {
  return js::IsProxy(obj) && js::sandbox::ProxyHandlerIsScripted(js::GetProxyHandler(obj));
}

}  // namespace mc

inline const js::BaseProxyHandler* MC_UNSAFE(const mc::BaseProxyHandler* handler) { return handler->UNSAFE_getProxyHandler(); }

namespace js {

inline JSObject* NewProxyObject(MCContext* cx,
                                const mc::BaseProxyHandler* handler,
                                JS::HandleValue priv, JSObject* proto,
                                const ProxyOptions& options = ProxyOptions()) {
  return NewProxyObject(cx->cx_, handler->UNSAFE_getProxyHandler(), priv, proto,
                        options);
}

//TODO(abhishek): REMOVE this overload
inline JSObject* NewProxyObject(JSContext* cx,
                                const mc::BaseProxyHandler* handler,
                                JS::HandleValue priv, JSObject* proto,
                                const ProxyOptions& options = ProxyOptions()) {
  return NewProxyObject(cx, handler->UNSAFE_getProxyHandler(), priv, proto,
                        options);
}

inline void assertEnteredPolicy(MCContext* cx, JSObject* obj, jsid id,
                                BaseProxyHandler::Action act) {
  return assertEnteredPolicy(cx->cx_, obj, id, act);
}

inline void NukeNonCCWProxy(MCContext* cx, JS::HandleObject proxy) {
  return NukeNonCCWProxy(cx->cx_, proxy);
}

inline void NukeRemovedCrossCompartmentWrapper(MCContext* cx,
                                               JSObject* wrapper) {
  return NukeRemovedCrossCompartmentWrapper(cx->cx_, wrapper);
}
}  // namespace js
#else
#  include "js/Proxy.h"

namespace mc {
using BaseProxyHandler = js::BaseProxyHandler;

// TODO(abhishek): remove once argument is Tainted
inline const BaseProxyHandler* GetProxyHandler(const JSObject* obj) {
  return js::GetProxyHandler(obj);
}

}

inline const js::BaseProxyHandler* MC_UNSAFE(const mc::BaseProxyHandler* handler) { return handler; }
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
