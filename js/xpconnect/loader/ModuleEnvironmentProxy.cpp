/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ModuleEnvironmentProxy.h"

#include "mozilla/Assertions.h"  // MOZ_ASSERT
#include "mozilla/Maybe.h"       // mozilla::Maybe

#include <stddef.h>  // size_t

#include "monkeycage/Class.h"        // JS::ObjectOpResult
#include "monkeycage/ErrorReport.h"  // JS_ReportOutOfMemory
#include "monkeycage/GCVector.h"     // MC::RootedVector
#include "js/Id.h"           // JS::PropertyKey
#include "monkeycage/PropertyAndElement.h"  // JS::IdVector, JS_HasPropertyById, JS_GetPropertyById, JS_Enumerate
#include "monkeycage/PropertyDescriptor.h"  // JS::PropertyDescriptor, JS_GetOwnPropertyDescriptorById
#include "monkeycage/Proxy.h"  // js::ProxyOptions, js::NewProxyObject, js::GetProxyPrivate
#include "monkeycage/RootingAPI.h"  // MC::Rooted, JS::Handle, JS::MutableHandle
#include "monkeycage/TypeDecls.h"   // JSContext, JSObject, JS::MutableHandleVector
#include "monkeycage/Value.h"       // JS::Value
#include "js/friend/ErrorMessages.h"  // JSMSG_*
#include "monkeycage/String.h"
#include "monkeycage/Modules.h"

namespace mozilla {
namespace loader {

struct ModuleEnvironmentProxyHandler : public mc::BaseProxyHandler {
  ModuleEnvironmentProxyHandler() : mc::BaseProxyHandler(&gFamily, false) {}

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
           JS::Handle<JS::Value> receiver, JS::Handle<JS::PropertyKey> aId,
           JS::MutableHandle<JS::Value> aVp) const override;
  bool ownPropertyKeys(
      MCContext* aCx, JS::Handle<JSObject*> aProxy,
      JS::MutableHandleVector<JS::PropertyKey> aProps) const override;

 private:
  static JSObject* getEnvironment(JS::Handle<JSObject*> aProxy) {
    return &js::GetProxyPrivate(aProxy).toObject();
  }

  static bool equalsNamespace(JSContext* aCx, JS::Handle<JS::PropertyKey> aId,
                              bool* aMatch) {
    if (!aId.isString()) {
      *aMatch = false;
      return true;
    }
    return JS_StringEqualsLiteral(aCx, aId.toString(), "*namespace*", aMatch);
  }

 public:
  static const char gFamily;
  static const ModuleEnvironmentProxyHandler* gHandler();
};

const ModuleEnvironmentProxyHandler* ModuleEnvironmentProxyHandler::gHandler() {
  static const ModuleEnvironmentProxyHandler inner_;
  return &inner_;  
}

const char ModuleEnvironmentProxyHandler::gFamily = 0;

bool ModuleEnvironmentProxyHandler::getOwnPropertyDescriptor(
    MCContext* aCx, JS::Handle<JSObject*> aProxy,
    JS::Handle<JS::PropertyKey> aId,
    JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> aDesc) const {
  bool isNamespace;
  if (!equalsNamespace(MC_UNSAFE(aCx), aId, &isNamespace)) {
    return false;
  }
  if (isNamespace) {
    aDesc.reset();
    return true;
  }

  MC::Rooted<JSObject*> envObj(aCx, getEnvironment(aProxy));
  if (!JS_GetOwnPropertyDescriptorById(aCx, envObj, aId, aDesc)) {
    return false;
  }

  if (aDesc.get().isNothing()) {
    return true;
  }

  JS::PropertyDescriptor& desc = *aDesc.get();

  desc.setConfigurable(false);
  desc.setWritable(false);
  desc.setEnumerable(true);

  return true;
}

bool ModuleEnvironmentProxyHandler::has(MCContext* aCx,
                                        JS::Handle<JSObject*> aProxy,
                                        JS::Handle<JS::PropertyKey> aId,
                                        MC::Tainted<bool*> aBp) const {
  bool isNamespace;
  if (!equalsNamespace(MC_UNSAFE(aCx), aId, &isNamespace)) {
    return false;
  }
  if (isNamespace) {
    *aBp = false;
    return true;
  }

  MC::Rooted<JSObject*> envObj(aCx, getEnvironment(aProxy));
  return JS_HasOwnPropertyById(aCx, envObj, aId, aBp);
}

bool ModuleEnvironmentProxyHandler::get(
    MCContext* aCx, JS::Handle<JSObject*> aProxy,
    JS::Handle<JS::Value> aReceiver, JS::Handle<JS::PropertyKey> aId,
    JS::MutableHandle<JS::Value> aVp) const {
  bool isNamespace;
  if (!equalsNamespace(MC_UNSAFE(aCx), aId, &isNamespace)) {
    return false;
  }
  if (isNamespace) {
    aVp.setUndefined();
    return true;
  }

  MC::Rooted<JSObject*> envObj(aCx, getEnvironment(aProxy));
  return JS_GetPropertyById(aCx, envObj, aId, aVp);
}

bool ModuleEnvironmentProxyHandler::ownPropertyKeys(
    MCContext* aCx, JS::Handle<JSObject*> aProxy,
    JS::MutableHandleVector<JS::PropertyKey> aProps) const {
  MC::Rooted<JSObject*> envObj(aCx, getEnvironment(aProxy));
  MC::Rooted<JS::IdVector> ids(aCx, JS::IdVector(MC_UNSAFE(aCx)));
  if (!JS_Enumerate(aCx, envObj, &ids)) {
    return false;
  }

  for (size_t i = 0; i < ids.length(); i++) {
    bool isNamespace;
    if (!equalsNamespace(MC_UNSAFE(aCx), ids[i], &isNamespace)) {
      return false;
    }
    if (isNamespace) {
      continue;
    }
    if (!aProps.append(ids[i])) {
      JS_ReportOutOfMemory(aCx);
      return false;
    }
  }

  return true;
}

JSObject* CreateModuleEnvironmentProxy(JSContext* aCx,
                                       JS::Handle<JSObject*> aModuleObj) {
  js::ProxyOptions options;
  options.setLazyProto(true);

  MC::Rooted<JSObject*> envObj(aCx, JS::GetModuleEnvironment(aCx, aModuleObj));
  if (!envObj) {
    return nullptr;
  }

  MC::Rooted<JS::Value> envVal(aCx, JS::ObjectValue(*envObj));
  return NewProxyObject(aCx, ModuleEnvironmentProxyHandler::gHandler(), envVal,
                        nullptr, options);
}

}  // namespace loader
}  // namespace mozilla
