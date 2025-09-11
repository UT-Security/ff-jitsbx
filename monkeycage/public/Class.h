/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JSClass definition and its component types, plus related interfaces. */

#ifndef mc_Class_h
#define mc_Class_h

#include "js/Class.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace MC {

namespace detail {

template <typename MC_Sbx>
class TaintedVolatile<JS::ObjectOpResult, MC_Sbx> {
 private:
  JS::ObjectOpResult data;

 public:
  bool ok() const { return data.ok(); }

  bool succeed() { return data.succeed(); }
  bool fail(uint32_t msg) { return data.fail(msg); }

  bool failCantRedefineProp() { return data.failCantRedefineProp(); }
  bool failReadOnly() { return data.failReadOnly(); }
  bool failGetterOnly() { return data.failGetterOnly(); }
  bool failCantDelete() { return data.failCantDelete(); }

  bool failCantSetInterposed() { return data.failCantSetInterposed(); }
  bool failCantDefineWindowElement() {
    return data.failCantDefineWindowElement();
  }
  bool failCantDeleteWindowElement() {
    return data.failCantDeleteWindowElement();
  }
  bool failCantDefineWindowNamedProperty() {
    return data.failCantDefineWindowNamedProperty();
  }
  bool failCantDeleteWindowNamedProperty() {
    return data.failCantDeleteWindowNamedProperty();
  }
  bool failCantPreventExtensions() { return data.failCantPreventExtensions(); }
  bool failCantSetProto() { return data.failCantSetProto(); }
  bool failNoNamedSetter() { return data.failNoNamedSetter(); }
  bool failNoIndexedSetter() { return data.failNoIndexedSetter(); }
  bool failNotDataDescriptor() { return data.failNotDataDescriptor(); }
  bool failInvalidDescriptor() { return data.failInvalidDescriptor(); }

  bool failCantDefineWindowNonConfigurable() {
    return data.failCantDefineWindowNonConfigurable();
  }

  bool failBadArrayLength() { return data.failBadArrayLength(); }
  bool failBadIndex() { return data.failBadIndex(); }

  bool reportError(MCContext* cx, JS::HandleObject obj, JS::HandleId id) {
    return data.reportError(cx->cx_, obj, id);
  }

  bool reportError(MCContext* cx, JS::HandleObject obj) {
    return data.reportError(cx->cx_, obj);
  }
};

template <typename MC_Sbx>
class TaintedVolatile<js::ElementAdder, MC_Sbx> {
 private:
  js::ElementAdder data;

 public:
  bool append(MCContext* cx, JS::HandleValue v) {
    return data.append(cx->cx_, v);
  }

  void appendHole() { return data.appendHole(); }
};

}  // namespace detail

}  // namespace MC

#endif

#endif
