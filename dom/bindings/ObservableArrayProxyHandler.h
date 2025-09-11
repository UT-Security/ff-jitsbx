/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ObservableArrayProxyHandler_h
#define mozilla_dom_ObservableArrayProxyHandler_h

#include "monkeycage/TypeDecls.h"
#include "monkeycage/Wrapper.h"

namespace mozilla::dom {

/**
 * Proxy handler for observable array exotic object.
 *
 * The indexed properties are stored in the backing list object in reserved slot
 * of the proxy object with special treatment intact.
 *
 * The additional properties are stored in the proxy target object.
 */

class ObservableArrayProxyHandler : public mc::ForwardingProxyHandler {
 public:
#ifdef JS_SANDBOX
  explicit inline ObservableArrayProxyHandler()
      : mc::ForwardingProxyHandler(&family) {}
#else
  explicit constexpr ObservableArrayProxyHandler()
      : mc::ForwardingProxyHandler(&family) {}
#endif

  // Implementations of methods that can be implemented in terms of
  // other lower-level methods.
  bool defineProperty(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                      JS::Handle<JS::PropertyKey> aId,
                      JS::Handle<JS::PropertyDescriptor> aDesc,
                      MC::Tainted<JS::ObjectOpResult*> aResult) const override;

  bool delete_(MCContext* aCx, JS::Handle<JSObject*> aProxy,
               JS::Handle<JS::PropertyKey> aId,
               MC::Tainted<JS::ObjectOpResult*> aResult) const override;

  bool get(MCContext* aCx, JS::Handle<JSObject*> aProxy,
           JS::Handle<JS::Value> aReceiver, JS::Handle<JS::PropertyKey> aId,
           JS::MutableHandle<JS::Value> aVp) const override;

  bool getOwnPropertyDescriptor(
      MCContext* aCx, JS::Handle<JSObject*> aProxy,
      JS::Handle<JS::PropertyKey> aId,
      JS::MutableHandle<Maybe<JS::PropertyDescriptor>> aDesc) const override;

  bool has(MCContext* aCx, JS::Handle<JSObject*> aProxy,
           JS::Handle<JS::PropertyKey> aId, MC::Tainted<bool*> aBp) const override;

  bool ownPropertyKeys(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                       JS::MutableHandleVector<jsid> aProps) const override;

  bool preventExtensions(
      MCContext* aCx, JS::Handle<JSObject*> aProxy,
      MC::Tainted<JS::ObjectOpResult*> aResult) const override;

  bool set(MCContext* aCx, JS::Handle<JSObject*> aProxy,
           JS::Handle<JS::PropertyKey> aId, JS::Handle<JS::Value> aV,
           JS::Handle<JS::Value> aReceiver,
           MC::Tainted<JS::ObjectOpResult*> aResult) const override;

  bool SetLength(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                 uint32_t aLength) const;

  static const char family;

 protected:
  bool GetBackingListObject(
      MCContext* aCx, JS::Handle<JSObject*> aProxy,
      JS::MutableHandle<JSObject*> aBackingListObject) const;

  bool GetBackingListLength(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                            MC::Tainted<uint32_t*> aLength) const;

  bool SetLength(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                 JS::Handle<JSObject*> aBackingList, uint32_t aLength,
                 MC::Tainted<JS::ObjectOpResult*> aResult) const;

  bool SetLength(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                 JS::Handle<JSObject*> aBackingList,
                 JS::Handle<JS::Value> aValue,
                 MC::Tainted<JS::ObjectOpResult*> aResult) const;

  // Hook for subclasses to invoke the setting the indexed value steps which
  // would invoke DeleteAlgorithm/SetAlgorithm defined and implemented per
  // interface. Returns false and throw exception on failure.
  virtual bool SetIndexedValue(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                               JS::Handle<JSObject*> aBackingList,
                               uint32_t aIndex, JS::Handle<JS::Value> aValue,
                               MC::Tainted<JS::ObjectOpResult*> aResult) const = 0;

  // Hook for subclasses to invoke the DeleteAlgorithm defined and implemented
  // per interface. Returns false and throw exception on failure.
  virtual bool OnDeleteItem(MCContext* aCx, JS::Handle<JSObject*> aProxy,
                            JS::Handle<JS::Value> aValue,
                            uint32_t aIndex) const = 0;
};

inline bool IsObservableArrayProxy(JSObject* obj) {
  return js::IsProxy(obj) && mc::GetProxyHandlerFamily(obj) ==
                                 &ObservableArrayProxyHandler::family;
}

inline const ObservableArrayProxyHandler* GetObservableArrayProxyHandler(
    JSObject* obj) {
  MOZ_ASSERT(IsObservableArrayProxy(obj));
  return static_cast<const ObservableArrayProxyHandler*>(
      mc::GetProxyHandler(obj));
}

}  // namespace mozilla::dom

#endif /* mozilla_dom_ObservableArrayProxyHandler_h */
