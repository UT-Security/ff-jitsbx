/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "JSMEnvironmentProxy.h"

#include "mozilla/Assertions.h"  // MOZ_ASSERT
#include "mozilla/Maybe.h"       // mozilla::Maybe

#include <stddef.h>  // size_t

#include "mcapi.h"  // JS_HasExtensibleLexicalEnvironment, JS_ExtensibleLexicalEnvironment
#include "monkeycage/Class.h"               // JS::ObjectOpResult
#include "monkeycage/ErrorReport.h"         // JS_ReportOutOfMemory
#include "monkeycage/GCVector.h"            // MC::RootedVector
#include "js/Id.h"                  // JS::PropertyKey
#include "monkeycage/PropertyAndElement.h"  // JS::IdVector, JS_HasPropertyById, JS_HasOwnPropertyById, JS_GetPropertyById, JS_Enumerate
#include "monkeycage/PropertyDescriptor.h"  // JS::PropertyDescriptor, JS_GetOwnPropertyDescriptorById
#include "monkeycage/Proxy.h"  // js::ProxyOptions, js::NewProxyObject, js::GetProxyPrivate
#include "monkeycage/RootingAPI.h"  // MC::Rooted, JS::Handle, JS::MutableHandle
#include "monkeycage/TypeDecls.h"   // JSContext, JSObject, JS::MutableHandleVector
#include "monkeycage/Value.h"  // JS::Value, JS::UndefinedValue, JS_UNINITIALIZED_LEXICAL
#include "js/friend/ErrorMessages.h"  // JSMSG_*

namespace mozilla {
namespace loader {

struct JSMEnvironmentProxyHandler : public mc::BaseProxyHandler {
  JSMEnvironmentProxyHandler() : BaseProxyHandler(&gFamily, false) {}

  bool defineProperty(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                      JS::Handle<JS::PropertyKey> aId,
                      JS::Handle<JS::PropertyDescriptor> aDesc,
                      MC::Tainted<JS::ObjectOpResult*> aResult) const override {
    return aResult->fail(JSMSG_CANT_DEFINE_PROP_OBJECT_NOT_EXTENSIBLE);
  }

  bool getPrototype(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                    JS::MutableHandle<JSObject*> aProtop) const override {
    aProtop.set(nullptr);
    return true;
  }

  bool setPrototype(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                    JS::Handle<JSObject*> aProto,
                    MC::Tainted<JS::ObjectOpResult*> aResult) const override {
    if (!aProto) {
      return aResult->succeed();
    }
    return aResult->failCantSetProto();
  }

  bool getPrototypeIfOrdinary(
      MCContext* aCx, JS::Handle<JSObject*> aProxy, MC::Tainted<bool*> aIsOrdinary,
      JS::MutableHandle<JSObject*> aProtop) const override {
    *aIsOrdinary = false;
    return true;
  }

  bool setImmutablePrototype(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                             MC::Tainted<bool*> aSucceeded) const override {
    *aSucceeded = true;
    return true;
  }

  bool preventExtensions(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                         MC::Tainted<JS::ObjectOpResult*> aResult) const override {
    aResult->succeed();
    return true;
  }

  bool isExtensible(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                    MC::Tainted<bool*> aExtensible) const override {
    *aExtensible = false;
    return true;
  }

  bool set(MCContext* aCx, JS::Handle<JSObject*> aProxy,
           JS::Handle<JS::PropertyKey> aId, JS::Handle<JS::Value> aValue,
           JS::Handle<JS::Value> aReceiver,
           MC::Tainted<JS::ObjectOpResult*> aResult) const override {
    return aResult->failReadOnly();
  }

  bool delete_(MCContext* aCx, JS::Handle<JSObject*> aProxy,
               JS::Handle<JS::PropertyKey> aId,
               MC::Tainted<JS::ObjectOpResult*> aResult) const override {
    return aResult->failCantDelete();
  }

  bool getOwnPropertyDescriptor(
      MCContext* aCx, JS::Handle<JSObject*> aProxy,
      JS::Handle<JS::PropertyKey> aId,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> aDesc)
      const override;
  bool has(MCContext* aCx, JS::Handle<JSObject*> aProxy,
           JS::Handle<JS::PropertyKey> aId, MC::Tainted<bool*> aBp) const override;
  bool get(MCContext* aCx, JS::Handle<JSObject*> aProxy,
           JS::Handle<JS::Value> aReceiver, JS::Handle<JS::PropertyKey> aId,
           JS::MutableHandle<JS::Value> aVp) const override;
  bool ownPropertyKeys(
      MCContext* aCx, JS::Handle<JSObject*> aProxy,
      JS::MutableHandleVector<JS::PropertyKey> aProps) const override;

 private:
  static JSObject* getGlobal(MCContext* aCx, JS::Handle<JSObject*> aProxy) {
    MC::Rooted<JSObject*> globalObj(aCx,
                                    &js::GetProxyPrivate(aProxy).toObject());
    return globalObj;
  }

 public:
  static const char gFamily;
  static const JSMEnvironmentProxyHandler* gHandler();
};

const JSMEnvironmentProxyHandler* JSMEnvironmentProxyHandler::gHandler() {
  static const JSMEnvironmentProxyHandler inner_;
  return &inner_;
}

const char JSMEnvironmentProxyHandler::gFamily = 0;

JSObject* ResolveModuleObjectPropertyById(MCContext* aCx,
                                          JS::Handle<JSObject*> aModObj,
                                          JS::Handle<JS::PropertyKey> aId) {
  if (JS_HasExtensibleLexicalEnvironment(aModObj)) {
    MC::Rooted<JSObject*> lexical(aCx,
                                  JS_ExtensibleLexicalEnvironment(aModObj));
    MC::SandboxStack<bool> found;
    if (!JS_HasOwnPropertyById(aCx, lexical, aId, found)) {
      return nullptr;
    }
    if (*found.UNSAFE_unverified()) {
      return lexical;
    }
  }
  return aModObj;
}

JSObject* ResolveModuleObjectProperty(MCContext* aCx,
                                      JS::Handle<JSObject*> aModObj,
                                      const char* aName) {
  if (JS_HasExtensibleLexicalEnvironment(aModObj)) {
    MC::RootedObject lexical(aCx, JS_ExtensibleLexicalEnvironment(aModObj));
    MC::SandboxStack<bool> found;
    if (!JS_HasOwnProperty(aCx, lexical, aName, found)) {
      return nullptr;
    }
    if (*found.UNSAFE_unverified()) {
      return lexical;
    }
  }
  return aModObj;
}

bool JSMEnvironmentProxyHandler::getOwnPropertyDescriptor(
    MCContext* aCx, JS::Handle<JSObject*> aProxy,
    JS::Handle<JS::PropertyKey> aId,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> aDesc) const {
  MC::Rooted<JSObject*> globalObj(aCx, getGlobal(aCx, aProxy));
  MC::Rooted<JSObject*> holder(
      aCx, ResolveModuleObjectPropertyById(aCx, globalObj, aId));
  if (!JS_GetOwnPropertyDescriptorById(aCx, holder, aId, aDesc)) {
    return false;
  }

  if (aDesc.get().isNothing()) {
    return true;
  }

  JS::PropertyDescriptor& desc = *aDesc.get();

  if (desc.hasValue()) {
    if (desc.value().isMagic(JS_UNINITIALIZED_LEXICAL)) {
      desc.setValue(JS::UndefinedValue());
    }
  }

  desc.setConfigurable(false);
  desc.setEnumerable(true);
  if (!desc.isAccessorDescriptor()) {
    desc.setWritable(false);
  }

  return true;
}

bool JSMEnvironmentProxyHandler::has(MCContext* aCx,
                                     JS::Handle<JSObject*> aProxy,
                                     JS::Handle<JS::PropertyKey> aId,
                                     MC::Tainted<bool*> aBp) const {
  MC::Rooted<JSObject*> globalObj(aCx, getGlobal(aCx, aProxy));
  MC::Rooted<JSObject*> holder(
      aCx, ResolveModuleObjectPropertyById(aCx, globalObj, aId));
  return JS_HasPropertyById(aCx, holder, aId, aBp);
}

bool JSMEnvironmentProxyHandler::get(MCContext* aCx,
                                     JS::Handle<JSObject*> aProxy,
                                     JS::Handle<JS::Value> aReceiver,
                                     JS::Handle<JS::PropertyKey> aId,
                                     JS::MutableHandle<JS::Value> aVp) const {
  MC::Rooted<JSObject*> globalObj(aCx, getGlobal(aCx, aProxy));
  MC::Rooted<JSObject*> holder(
      aCx, ResolveModuleObjectPropertyById(aCx, globalObj, aId));
  if (!JS_GetPropertyById(aCx, holder, aId, aVp)) {
    return false;
  }

  if (aVp.isMagic(JS_UNINITIALIZED_LEXICAL)) {
    aVp.setUndefined();
  }

  return true;
}

bool JSMEnvironmentProxyHandler::ownPropertyKeys(
    MCContext* aCx, JS::Handle<JSObject*> aProxy,
    JS::MutableHandleVector<JS::PropertyKey> aProps) const {
  MC::Rooted<JSObject*> globalObj(aCx, getGlobal(aCx, aProxy));
  MC::Rooted<JS::IdVector> globalIds(aCx, JS::IdVector(MC_UNSAFE(aCx)));
  if (!JS_Enumerate(aCx, globalObj, &globalIds)) {
    return false;
  }

  for (size_t i = 0; i < globalIds.length(); i++) {
    if (!aProps.append(globalIds[i])) {
      JS_ReportOutOfMemory(aCx);
      return false;
    }
  }

  MC::RootedObject lexicalEnv(aCx, JS_ExtensibleLexicalEnvironment(globalObj));
  MC::Rooted<JS::IdVector> lexicalIds(aCx, JS::IdVector(MC_UNSAFE(aCx)));
  if (!JS_Enumerate(aCx, lexicalEnv, &lexicalIds)) {
    return false;
  }

  for (size_t i = 0; i < lexicalIds.length(); i++) {
    if (!aProps.append(lexicalIds[i])) {
      JS_ReportOutOfMemory(aCx);
      return false;
    }
  }

  return true;
}

JSObject* CreateJSMEnvironmentProxy(MCContext* aCx,
                                    JS::Handle<JSObject*> aGlobalObj) {
  js::ProxyOptions options;
  options.setLazyProto(true);

  MC::Rooted<JS::Value> globalVal(aCx, JS::ObjectValue(*aGlobalObj));
  return NewProxyObject(aCx, JSMEnvironmentProxyHandler::gHandler(), globalVal,
                        nullptr, options);
}

}  // namespace loader
}  // namespace mozilla
