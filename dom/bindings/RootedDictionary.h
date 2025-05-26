/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_RootedDictionary_h__
#define mozilla_dom_RootedDictionary_h__

#include "mozilla/dom/Nullable.h"
#include "mcapi.h"

namespace mozilla::dom {

template <typename T>
class MOZ_RAII RootedDictionary final : public T, private MC::CustomAutoRooter {
 public:
  template <typename CX>
  explicit RootedDictionary(const CX& cx) : T(), MC::CustomAutoRooter(cx) {}

  virtual void trace(JSTracer* trc) override { this->TraceDictionary(trc); }
};

template <typename T>
class MOZ_RAII NullableRootedDictionary final : public Nullable<T>,
                                                private MC::CustomAutoRooter {
 public:
  template <typename CX>
  explicit NullableRootedDictionary(const CX& cx)
      : Nullable<T>(), MC::CustomAutoRooter(cx) {}

  virtual void trace(JSTracer* trc) override {
    if (!this->IsNull()) {
      this->Value().TraceDictionary(trc);
    }
  }
};

}  // namespace mozilla::dom

#endif /* mozilla_dom_RootedDictionary_h__ */
