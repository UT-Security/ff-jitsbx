/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Proxy_h
#define js_sandbox_Proxy_h

#include "js/Proxy.h"

namespace js {
namespace sandbox {

typedef bool (*ProxyFinalizeInBackgroundOp)(const void* p,
                                            const JS::Value& priv);

typedef bool (*ProxyCanNurseryAllocateOp)(const void* p);

typedef bool (*ProxyEnterOp)(const void* p, JSContext* cx,
                             JS::HandleObject wrapper, JS::HandleId id,
                             js::BaseProxyHandler::Action act, bool mayThrow,
                             bool* bp);

typedef bool (*ProxyGetOwnPropertyDescriptorOp)(
    const void* p, JSContext* cx, JS::HandleObject proxy, JS::HandleId id,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc);

typedef bool (*ProxyDefinePropertyOp)(const void* p, JSContext* cx,
                                      JS::HandleObject proxy, JS::HandleId id,
                                      JS::Handle<JS::PropertyDescriptor> desc,
                                      JS::ObjectOpResult& result);

typedef bool (*ProxyOwnPropertyKeysOp)(const void* p, JSContext* cx,
                                       JS::HandleObject proxy,
                                       JS::MutableHandleIdVector props);

typedef bool (*ProxyDeleteOp)(const void* p, JSContext* cx,
                              JS::HandleObject proxy, JS::HandleId id,
                              JS::ObjectOpResult& result);

typedef bool (*ProxyGetPrototypeOp)(const void* p, JSContext* cx,
                                    JS::HandleObject proxy,
                                    JS::MutableHandleObject protop);

typedef bool (*ProxySetPrototypeOp)(const void* p, JSContext* cx,
                                    JS::HandleObject proxy,
                                    JS::HandleObject proto,
                                    JS::ObjectOpResult& result);

typedef bool (*ProxyGetPrototypeIfOrdinaryOp)(const void* p, JSContext* cx,
                                              JS::HandleObject proxy,
                                              bool* isOrdinary,
                                              JS::MutableHandleObject protop);

typedef bool (*ProxySetImmutablePrototypeOp)(const void* p, JSContext* cx,
                                             JS::HandleObject proxy,
                                             bool* succeeded);

typedef bool (*ProxyPreventExtensionsOp)(const void* p, JSContext* cx,
                                         JS::HandleObject proxy,
                                         JS::ObjectOpResult& result);

typedef bool (*ProxyIsExtensibleOp)(const void* p, JSContext* cx,
                                    JS::HandleObject proxy, bool* extensible);

typedef bool (*ProxyHasOp)(const void* p, JSContext* cx, JS::HandleObject proxy,
                           JS::HandleId id, bool* bp);

typedef bool (*ProxyGetOp)(const void* p, JSContext* cx, JS::HandleObject proxy,
                           JS::HandleValue receiver, JS::HandleId id,
                           JS::MutableHandleValue vp);

typedef bool (*ProxySetOp)(const void* p, JSContext* cx, JS::HandleObject proxy,
                           JS::HandleId id, JS::HandleValue v,
                           JS::HandleValue receiver,
                           JS::ObjectOpResult& result);

typedef bool (*ProxyUseProxyExpandoObjectForPrivateFieldsOp)(const void* p);

typedef bool (*ProxyThrowOnPrivateFieldOp)(const void* p);

typedef bool (*ProxyCallOp)(const void* p, JSContext* cx,
                            JS::HandleObject proxy, const JS::CallArgs& args);

typedef bool (*ProxyConstructOp)(const void* p, JSContext* cx,
                                 JS::HandleObject proxy,
                                 const JS::CallArgs& args);

typedef bool (*ProxyEnumerateOp)(const void* p, JSContext* cx,
                                 JS::HandleObject proxy,
                                 JS::MutableHandleIdVector props);

typedef bool (*ProxyHasOwnOp)(const void* p, JSContext* cx,
                              JS::HandleObject proxy, JS::HandleId id,
                              bool* bp);

typedef bool (*ProxyGetOwnEnumerablePropertyKeysOp)(
    const void* p, JSContext* cx, JS::HandleObject proxy,
    JS::MutableHandleIdVector props);

typedef bool (*ProxyNativeCallOp)(const void* p, JSContext* cx,
                                  JS::IsAcceptableThis test,
                                  JS::NativeImpl impl,
                                  const JS::CallArgs& args);

typedef bool (*ProxyGetBuiltinClassOp)(const void* p, JSContext* cx,
                                       JS::HandleObject proxy, ESClass* cls);

typedef bool (*ProxyIsArrayOp)(const void* p, JSContext* cx,
                               JS::HandleObject proxy,
                               JS::IsArrayAnswer* answer);

typedef const char* (*ProxyClassNameOp)(const void* p, JSContext* cx,
                                        JS::HandleObject proxy);

typedef JSString* (*ProxyFunToStringOp)(const void* p, JSContext* cx,
                                        JS::HandleObject proxy,
                                        bool isToSource);

typedef RegExpShared* (*ProxyRegexpToSharedOp)(const void* p, JSContext* cx,
                                               JS::HandleObject proxy);

typedef bool (*ProxyBoxedValueUnboxOp)(const void* p, JSContext* cx,
                                       JS::HandleObject proxy,
                                       JS::MutableHandleValue vp);

typedef void (*ProxyTraceOp)(const void* p, JSTracer* trc, JSObject* proxy);

typedef void (*ProxyFinalizeOp)(const void* p, JS::GCContext* gcx,
                                JSObject* proxy);

typedef size_t (*ProxyObjectMovedOp)(const void* p, JSObject* proxy,
                                     JSObject* old);

typedef bool (*ProxyIsCallableOp)(const void* p, JSObject* obj);

typedef bool (*ProxyIsConstructorOp)(const void* p, JSObject* obj);

typedef bool (*ProxyGetElementsOp)(const void* p, JSContext* cx,
                                   JS::HandleObject proxy, uint32_t begin,
                                   uint32_t end, ElementAdder* adder);

typedef bool (*ProxyIsScriptedOp)(const void* p);

struct JS_PUBLIC_API ProxyHandlerOps {
  ProxyFinalizeInBackgroundOp finalizeInBackground;
  ProxyCanNurseryAllocateOp canNurseryAllocate;
  ProxyEnterOp enter;
  ProxyGetOwnPropertyDescriptorOp getOwnPropertyDescriptor;
  ProxyDefinePropertyOp defineProperty;
  ProxyOwnPropertyKeysOp ownPropertyKeys;
  ProxyDeleteOp delete_;
  ProxyGetPrototypeOp getPrototype;
  ProxySetPrototypeOp setPrototype;
  ProxyGetPrototypeIfOrdinaryOp getPrototypeIfOrdinary;
  ProxySetImmutablePrototypeOp setImmutablePrototype;
  ProxyPreventExtensionsOp preventExtensions;
  ProxyIsExtensibleOp isExtensible;
  ProxyHasOp has;
  ProxyGetOp get;
  ProxySetOp set;
  ProxyUseProxyExpandoObjectForPrivateFieldsOp
      useProxyExpandoObjectForPrivateFields;
  ProxyThrowOnPrivateFieldOp throwOnPrivateField;
  ProxyCallOp call;
  ProxyConstructOp construct;
  ProxyEnumerateOp enumerate;
  ProxyHasOwnOp hasOwn;
  ProxyGetOwnEnumerablePropertyKeysOp getOwnEnumerablePropertyKeys;
  ProxyNativeCallOp nativeCall;
  ProxyGetBuiltinClassOp getBuiltinClass;
  ProxyIsArrayOp isArray;
  ProxyClassNameOp className;
  ProxyFunToStringOp fun_toString;
  ProxyRegexpToSharedOp regexp_toShared;
  ProxyBoxedValueUnboxOp boxedValue_unbox;
  ProxyTraceOp trace;
  ProxyFinalizeOp finalize;
  ProxyObjectMovedOp objectMoved;
  ProxyIsCallableOp isCallable;
  ProxyIsConstructorOp isConstructor;
  ProxyGetElementsOp getElements;
  ProxyIsScriptedOp isScripted;
};

#define DECLARE_PROXY_HANDLER_OPS_METHODS                                        \
  bool finalizeInBackground(const JS::Value& priv) const override;               \
  bool canNurseryAllocate() const override;                                      \
  bool enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,           \
             Action act, bool mayThrow, bool* bp) const override;                \
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
  bool isScripted() const override;

#define DEFINE_PROXY_HANDLER_OPS_METHODS(name, ops, handler)                     \
bool name::finalizeInBackground(const JS::Value& priv) const {                   \
  return ops->finalizeInBackground(handler, priv);                               \
}                                                                                \
bool name::canNurseryAllocate() const {                                          \
  return ops->canNurseryAllocate(handler);                                       \
}                                                                                \
bool name::enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,       \
                 Action act, bool mayThrow, bool* bp) const {                    \
  return ops->enter(handler, cx, wrapper, id, act, mayThrow, bp);                \
}                                                                                \
bool name::getOwnPropertyDescriptor(                                             \
    JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                      \
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {      \
  return ops->getOwnPropertyDescriptor(handler, cx, proxy, id, desc);            \
}                                                                                \
bool name::defineProperty(JSContext* cx, JS::HandleObject proxy,                 \
                          JS::HandleId id,                                       \
                          JS::Handle<JS::PropertyDescriptor> desc,               \
                          JS::ObjectOpResult& result) const {                    \
  return ops->defineProperty(handler, cx, proxy, id, desc, result);              \
}                                                                                \
bool name::ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,                \
                           JS::MutableHandleIdVector props) const {              \
  return ops->ownPropertyKeys(handler, cx, proxy, props);                        \
}                                                                                \
bool name::delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,       \
                   JS::ObjectOpResult& result) const {                           \
  return ops->delete_(handler, cx, proxy, id, result);                           \
}                                                                                \
bool name::getPrototype(JSContext* cx, JS::HandleObject proxy,                   \
                        JS::MutableHandleObject protop) const {                  \
  return ops->getPrototype(handler, cx, proxy, protop);                          \
}                                                                                \
bool name::setPrototype(JSContext* cx, JS::HandleObject proxy,                   \
                        JS::HandleObject proto,                                  \
                        JS::ObjectOpResult& result) const {                      \
  return ops->setPrototype(handler, cx, proxy, proto, result);                   \
}                                                                                \
bool name::getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,         \
                                  bool* isOrdinary,                              \
                                  JS::MutableHandleObject protop) const {        \
  return ops->getPrototypeIfOrdinary(handler, cx, proxy, isOrdinary, protop);    \
}                                                                                \
bool name::setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,          \
                                 bool* succeeded) const {                        \
  return ops->setImmutablePrototype(handler, cx, proxy, succeeded);              \
}                                                                                \
bool name::preventExtensions(JSContext* cx, JS::HandleObject proxy,              \
                             JS::ObjectOpResult& result) const {                 \
  return ops->preventExtensions(handler, cx, proxy, result);                     \
}                                                                                \
bool name::isExtensible(JSContext* cx, JS::HandleObject proxy,                   \
                        bool* extensible) const {                                \
  return ops->isExtensible(handler, cx, proxy, extensible);                      \
}                                                                                \
bool name::has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,           \
               bool* bp) const {                                                 \
  return ops->has(handler, cx, proxy, id, bp);                                   \
}                                                                                \
bool name::get(JSContext* cx, JS::HandleObject proxy, JS::HandleValue receiver,  \
               JS::HandleId id, JS::MutableHandleValue vp) const {               \
  return ops->get(handler, cx, proxy, receiver, id, vp);                         \
}                                                                                \
bool name::set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,           \
               JS::HandleValue v, JS::HandleValue receiver,                      \
               JS::ObjectOpResult& result) const {                               \
  return ops->set(handler, cx, proxy, id, v, receiver, result);                  \
}                                                                                \
bool name::useProxyExpandoObjectForPrivateFields() const {                       \
  return ops->useProxyExpandoObjectForPrivateFields(handler);                    \
}                                                                                \
bool name::throwOnPrivateField() const {                                         \
  return ops->throwOnPrivateField(handler);                                      \
}                                                                                \
bool name::call(JSContext* cx, JS::HandleObject proxy,                           \
                const JS::CallArgs& args) const {                                \
  return ops->call(handler, cx, proxy, args);                                    \
}                                                                                \
bool name::construct(JSContext* cx, JS::HandleObject proxy,                      \
                     const JS::CallArgs& args) const {                           \
  return ops->construct(handler, cx, proxy, args);                               \
}                                                                                \
bool name::enumerate(JSContext* cx, JS::HandleObject proxy,                      \
                     JS::MutableHandleIdVector props) const {                    \
  return ops->enumerate(handler, cx, proxy, props);                              \
}                                                                                \
bool name::hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,        \
                  bool* bp) const {                                              \
  return ops->hasOwn(handler, cx, proxy, id, bp);                                \
}                                                                                \
bool name::getOwnEnumerablePropertyKeys(JSContext* cx, JS::HandleObject proxy,   \
                                        JS::MutableHandleIdVector props) const { \
  return ops->getOwnEnumerablePropertyKeys(handler, cx, proxy, props);           \
}                                                                                \
bool name::nativeCall(JSContext* cx, JS::IsAcceptableThis test,                  \
                      JS::NativeImpl impl, const JS::CallArgs& args) const {     \
  return ops->nativeCall(handler, cx, test, impl, args);                         \
}                                                                                \
bool name::getBuiltinClass(JSContext* cx, JS::HandleObject proxy,                \
                           ESClass* cls) const {                                 \
  return ops->getBuiltinClass(handler, cx, proxy, cls);                          \
}                                                                                \
bool name::isArray(JSContext* cx, JS::HandleObject proxy,                        \
                   JS::IsArrayAnswer* answer) const {                            \
  return ops->isArray(handler, cx, proxy, answer);                               \
}                                                                                \
const char* name::className(JSContext* cx, JS::HandleObject proxy) const {       \
  return ops->className(handler, cx, proxy);                                     \
}                                                                                \
JSString* name::fun_toString(JSContext* cx, JS::HandleObject proxy,              \
                             bool isToSource) const {                            \
  return ops->fun_toString(handler, cx, proxy, isToSource);                      \
}                                                                                \
RegExpShared* name::regexp_toShared(JSContext* cx,                               \
                                    JS::HandleObject proxy) const {              \
  return ops->regexp_toShared(handler, cx, proxy);                               \
}                                                                                \
bool name::boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,               \
                            JS::MutableHandleValue vp) const {                   \
  return ops->boxedValue_unbox(handler, cx, proxy, vp);                          \
}                                                                                \
void name::trace(JSTracer* trc, JSObject* proxy) const {                         \
  return ops->trace(handler, trc, proxy);                                        \
}                                                                                \
void name::finalize(JS::GCContext* gcx, JSObject* proxy) const {                 \
  return ops->finalize(handler, gcx, proxy);                                     \
}                                                                                \
size_t name::objectMoved(JSObject* proxy, JSObject* old) const {                 \
  return ops->objectMoved(handler, proxy, old);                                  \
}                                                                                \
bool name::isCallable(JSObject* obj) const {                                     \
  return ops->isCallable(handler, obj);                                          \
}                                                                                \
bool name::isConstructor(JSObject* obj) const {                                  \
  return ops->isConstructor(handler, obj);                                       \
}                                                                                \
bool name::getElements(JSContext* cx, JS::HandleObject proxy, uint32_t begin,    \
                       uint32_t end, ElementAdder* adder) const {                \
  return ops->getElements(handler, cx, proxy, begin, end, adder);                \
}                                                                                \
bool name::isScripted() const { return ops->isScripted(handler); }

class JS_PUBLIC_API BaseProxyHandler : public js::BaseProxyHandler {
  const ProxyHandlerOps* ops_;
  const void* handler_;

public:
 explicit BaseProxyHandler(const ProxyHandlerOps* ops, const void* handler,
                           const void* aFamily, bool aHasPrototype = false,
                           bool aHasSecurityPolicy = false);

 inline const void* getHandler() const { return handler_; }

 DECLARE_PROXY_HANDLER_OPS_METHODS
};

}  // namespace sandbox
}  // namespace js

#endif
