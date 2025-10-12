/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JS::Value implementation. */

#ifndef mc_Value_h
#define mc_Value_h

#include "js/Value.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/StoreBuffer.h"

namespace MC {
const JS::HandleValue& NullHandleValue();
const JS::HandleValue& UndefinedHandleValue();
const JS::HandleValue& TrueHandleValue();
const JS::HandleValue& FalseHandleValue();
const JS::Handle<mozilla::Maybe<JS::Value>>& NothingHandleValue();
}

namespace mc {

template <>
struct BarrierMethods<JS::Value> {
  static js::gc::Cell* asGCThingOrNull(const JS::Value& v) {
    return v.isGCThing() ? v.toGCThing() : nullptr;
  }
  static void postWriteBarrier(JS::Value* v, const JS::Value& prev,
                               const JS::Value& next) {
    MC::HeapValuePostWriteBarrier(v, prev, next);
  }
  static void exposeToJS(const JS::Value& v) { JS::ExposeValueToActiveJS(v); }
  static void readBarrier(const JS::Value& v) {
    if (v.isGCThing()) {
      js::gc::IncrementalReadBarrier(v.toGCCellPtr());
    }
  }
};

}

#endif
