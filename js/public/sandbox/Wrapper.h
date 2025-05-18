/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Wrapper_h
#define js_sandbox_Wrapper_h

#include "js/Wrapper.h"
#include "js/sandbox/Proxy.h"

namespace js {
namespace sandbox {

class JS_PUBLIC_API ForwardingProxyHandler : public js::ForwardingProxyHandler {
  const ProxyHandlerOps* ops_;
  const void* handler_;

public:
 explicit ForwardingProxyHandler(const ProxyHandlerOps* ops, const void* handler,
                           const void* aFamily, bool aHasPrototype = false,
                           bool aHasSecurityPolicy = false);

 inline const void* getHandler() const { return handler_; }

 DECLARE_PROXY_HANDLER_OPS_METHODS
};

typedef bool (*WrapperDynamicCheckedUnwrapAllowedOp)(const void* p, JS::HandleObject obj, JSContext* cx);

struct JS_PUBLIC_API WrapperOps : public ProxyHandlerOps {
  WrapperDynamicCheckedUnwrapAllowedOp dynamicCheckedUnwrapAllowed;
};

#define DECLARE_WRAPPER_OPS_METHODS                                         \
  DECLARE_PROXY_HANDLER_OPS_METHODS                                         \
  virtual bool dynamicCheckedUnwrapAllowed(JS::HandleObject obj,            \
                                           JSContext* cx) const override;

#define DEFINE_WRAPPER_OPS_METHODS(name, ops, wrapper)                      \
  DEFINE_PROXY_HANDLER_OPS_METHODS(name, ops, wrapper)                      \
  bool name::dynamicCheckedUnwrapAllowed(JS::HandleObject obj,              \
                                         JSContext* cx) const {             \
    return ops->dynamicCheckedUnwrapAllowed(wrapper, obj, cx);              \
  }

class JS_PUBLIC_API Wrapper : public js::Wrapper {
  const WrapperOps* ops_;
  const void* wrapper_;

 public:
  explicit Wrapper(const WrapperOps* ops, const void* wrapper, unsigned aFlags,
                   bool aHasPrototype = false, bool aHasSecurityPolicy = false);

  inline const void* getWrapper() const { return wrapper_; }

  DECLARE_WRAPPER_OPS_METHODS
};

class JS_PUBLIC_API CrossCompartmentWrapper
    : public js::CrossCompartmentWrapper {
  const WrapperOps* ops_;
  const void* wrapper_;

 public:
  explicit CrossCompartmentWrapper(const WrapperOps* ops, const void* wrapper,
                                   unsigned aFlags, bool aHasPrototype = false,
                                   bool aHasSecurityPolicy = false);

  inline const void* getWrapper() const { return wrapper_; }

  DECLARE_WRAPPER_OPS_METHODS
};

class JS_PUBLIC_API OpaqueCrossCompartmentWrapper
    : public js::OpaqueCrossCompartmentWrapper {
  const WrapperOps* ops_;
  const void* wrapper_;

 public:
  explicit OpaqueCrossCompartmentWrapper(const WrapperOps* ops,
                                         const void* wrapper);
  inline const void* getWrapper() const { return wrapper_; }

  DECLARE_WRAPPER_OPS_METHODS
};


#define DEFINE_SECURITY_WRAPPER_OPS_METHODS(name, ops, handler)                  \
template <class Base>                                                            \
bool name::finalizeInBackground(const JS::Value& priv) const {                   \
  return ops->finalizeInBackground(handler, priv);                               \
}                                                                                \
template <class Base>                                                            \
bool name::canNurseryAllocate() const {                                          \
  return ops->canNurseryAllocate(handler);                                       \
}                                                                                \
template <class Base>                                                            \
bool name::enter(JSContext* cx, JS::HandleObject wrapper, JS::HandleId id,       \
                 BaseProxyHandler::Action act, bool mayThrow, bool* bp) const {  \
  return ops->enter(handler, cx, wrapper, id, act, mayThrow, bp);                \
}                                                                                \
template <class Base>                                                            \
bool name::getOwnPropertyDescriptor(                                             \
    JSContext* cx, JS::HandleObject proxy, JS::HandleId id,                      \
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc) const {      \
  return ops->getOwnPropertyDescriptor(handler, cx, proxy, id, desc);            \
}                                                                                \
template <class Base>                                                            \
bool name::defineProperty(JSContext* cx, JS::HandleObject proxy,                 \
                          JS::HandleId id,                                       \
                          JS::Handle<JS::PropertyDescriptor> desc,               \
                          JS::ObjectOpResult& result) const {                    \
  return ops->defineProperty(handler, cx, proxy, id, desc, result);              \
}                                                                                \
template <class Base>                                                            \
bool name::ownPropertyKeys(JSContext* cx, JS::HandleObject proxy,                \
                           JS::MutableHandleIdVector props) const {              \
  return ops->ownPropertyKeys(handler, cx, proxy, props);                        \
}                                                                                \
template <class Base>                                                            \
bool name::delete_(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,       \
                   JS::ObjectOpResult& result) const {                           \
  return ops->delete_(handler, cx, proxy, id, result);                           \
}                                                                                \
template <class Base>                                                            \
bool name::getPrototype(JSContext* cx, JS::HandleObject proxy,                   \
                        JS::MutableHandleObject protop) const {                  \
  return ops->getPrototype(handler, cx, proxy, protop);                          \
}                                                                                \
template <class Base>                                                            \
bool name::setPrototype(JSContext* cx, JS::HandleObject proxy,                   \
                        JS::HandleObject proto,                                  \
                        JS::ObjectOpResult& result) const {                      \
  return ops->setPrototype(handler, cx, proxy, proto, result);                   \
}                                                                                \
template <class Base>                                                            \
bool name::getPrototypeIfOrdinary(JSContext* cx, JS::HandleObject proxy,         \
                                  bool* isOrdinary,                              \
                                  JS::MutableHandleObject protop) const {        \
  return ops->getPrototypeIfOrdinary(handler, cx, proxy, isOrdinary, protop);    \
}                                                                                \
template <class Base>                                                            \
bool name::setImmutablePrototype(JSContext* cx, JS::HandleObject proxy,          \
                                 bool* succeeded) const {                        \
  return ops->setImmutablePrototype(handler, cx, proxy, succeeded);              \
}                                                                                \
template <class Base>                                                            \
bool name::preventExtensions(JSContext* cx, JS::HandleObject proxy,              \
                             JS::ObjectOpResult& result) const {                 \
  return ops->preventExtensions(handler, cx, proxy, result);                     \
}                                                                                \
template <class Base>                                                            \
bool name::isExtensible(JSContext* cx, JS::HandleObject proxy,                   \
                        bool* extensible) const {                                \
  return ops->isExtensible(handler, cx, proxy, extensible);                      \
}                                                                                \
template <class Base>                                                            \
bool name::has(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,           \
               bool* bp) const {                                                 \
  return ops->has(handler, cx, proxy, id, bp);                                   \
}                                                                                \
template <class Base>                                                            \
bool name::get(JSContext* cx, JS::HandleObject proxy, JS::HandleValue receiver,  \
               JS::HandleId id, JS::MutableHandleValue vp) const {               \
  return ops->get(handler, cx, proxy, receiver, id, vp);                         \
}                                                                                \
template <class Base>                                                            \
bool name::set(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,           \
               JS::HandleValue v, JS::HandleValue receiver,                      \
               JS::ObjectOpResult& result) const {                               \
  return ops->set(handler, cx, proxy, id, v, receiver, result);                  \
}                                                                                \
template <class Base>                                                            \
bool name::useProxyExpandoObjectForPrivateFields() const {                       \
  return ops->useProxyExpandoObjectForPrivateFields(handler);                    \
}                                                                                \
template <class Base>                                                            \
bool name::throwOnPrivateField() const {                                         \
  return ops->throwOnPrivateField(handler);                                      \
}                                                                                \
template <class Base>                                                            \
bool name::call(JSContext* cx, JS::HandleObject proxy,                           \
                const JS::CallArgs& args) const {                                \
  return ops->call(handler, cx, proxy, args);                                    \
}                                                                                \
template <class Base>                                                            \
bool name::construct(JSContext* cx, JS::HandleObject proxy,                      \
                     const JS::CallArgs& args) const {                           \
  return ops->construct(handler, cx, proxy, args);                               \
}                                                                                \
template <class Base>                                                            \
bool name::enumerate(JSContext* cx, JS::HandleObject proxy,                      \
                     JS::MutableHandleIdVector props) const {                    \
  return ops->enumerate(handler, cx, proxy, props);                              \
}                                                                                \
template <class Base>                                                            \
bool name::hasOwn(JSContext* cx, JS::HandleObject proxy, JS::HandleId id,        \
                  bool* bp) const {                                              \
  return ops->hasOwn(handler, cx, proxy, id, bp);                                \
}                                                                                \
template <class Base>                                                            \
bool name::getOwnEnumerablePropertyKeys(JSContext* cx, JS::HandleObject proxy,   \
                                        JS::MutableHandleIdVector props) const { \
  return ops->getOwnEnumerablePropertyKeys(handler, cx, proxy, props);           \
}                                                                                \
template <class Base>                                                            \
bool name::nativeCall(JSContext* cx, JS::IsAcceptableThis test,                  \
                      JS::NativeImpl impl, const JS::CallArgs& args) const {     \
  return ops->nativeCall(handler, cx, test, impl, args);                         \
}                                                                                \
template <class Base>                                                            \
bool name::getBuiltinClass(JSContext* cx, JS::HandleObject proxy,                \
                           ESClass* cls) const {                                 \
  return ops->getBuiltinClass(handler, cx, proxy, cls);                          \
}                                                                                \
template <class Base>                                                            \
bool name::isArray(JSContext* cx, JS::HandleObject proxy,                        \
                   JS::IsArrayAnswer* answer) const {                            \
  return ops->isArray(handler, cx, proxy, answer);                               \
}                                                                                \
template <class Base>                                                            \
const char* name::className(JSContext* cx, JS::HandleObject proxy) const {       \
  return ops->className(handler, cx, proxy);                                     \
}                                                                                \
template <class Base>                                                            \
JSString* name::fun_toString(JSContext* cx, JS::HandleObject proxy,              \
                             bool isToSource) const {                            \
  return ops->fun_toString(handler, cx, proxy, isToSource);                      \
}                                                                                \
template <class Base>                                                            \
js::RegExpShared* name::regexp_toShared(JSContext* cx,                           \
                                    JS::HandleObject proxy) const {              \
  return ops->regexp_toShared(handler, cx, proxy);                               \
}                                                                                \
template <class Base>                                                            \
bool name::boxedValue_unbox(JSContext* cx, JS::HandleObject proxy,               \
                            JS::MutableHandleValue vp) const {                   \
  return ops->boxedValue_unbox(handler, cx, proxy, vp);                          \
}                                                                                \
template <class Base>                                                            \
void name::trace(JSTracer* trc, JSObject* proxy) const {                         \
  return ops->trace(handler, trc, proxy);                                        \
}                                                                                \
template <class Base>                                                            \
void name::finalize(JS::GCContext* gcx, JSObject* proxy) const {                 \
  return ops->finalize(handler, gcx, proxy);                                     \
}                                                                                \
template <class Base>                                                            \
size_t name::objectMoved(JSObject* proxy, JSObject* old) const {                 \
  return ops->objectMoved(handler, proxy, old);                                  \
}                                                                                \
template <class Base>                                                            \
bool name::isCallable(JSObject* obj) const {                                     \
  return ops->isCallable(handler, obj);                                          \
}                                                                                \
template <class Base>                                                            \
bool name::isConstructor(JSObject* obj) const {                                  \
  return ops->isConstructor(handler, obj);                                       \
}                                                                                \
template <class Base>                                                            \
bool name::getElements(JSContext* cx, JS::HandleObject proxy, uint32_t begin,    \
                       uint32_t end, ElementAdder* adder) const {                \
  return ops->getElements(handler, cx, proxy, begin, end, adder);                \
}                                                                                \
template <class Base>                                                            \
bool name::isScripted() const { return ops->isScripted(handler); }               \
template <class Base>                                                            \
bool name::dynamicCheckedUnwrapAllowed(JS::HandleObject obj,                     \
                                       JSContext* cx) const {                    \
  return ops->dynamicCheckedUnwrapAllowed(handler, obj, cx);                     \
}

template <class Base>
class JS_PUBLIC_API SecurityWrapper : public js::SecurityWrapper<Base> {
  const WrapperOps* ops_;
  const void* wrapper_;

public:
  explicit SecurityWrapper(const WrapperOps* ops, const void* wrapper, unsigned flags, bool hasPrototype = false);

  inline const void* getWrapper() const { return wrapper_; }

  DECLARE_WRAPPER_OPS_METHODS
};

typedef SecurityWrapper<js::CrossCompartmentWrapper> CrossCompartmentSecurityWrapper;

}  // namespace sandbox
}  // namespace js
#endif
