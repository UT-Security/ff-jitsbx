/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Options specified when creating a realm to determine its behavior, immutable
 * options determining the behavior of an existing realm, and mutable options on
 * an existing realm that may be changed when desired.
 */

#ifndef mc_RealmOptions_h
#define mc_RealmOptions_h

#include "js/RealmOptions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Tainted.h"

namespace MC {
namespace detail {

template <typename MC_Sbx>
class TaintedVolatile<JS::RealmCreationOptions, MC_Sbx> {
private:
  JS::RealmCreationOptions data;

public:
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setTrace(MC::SandboxCallback<JSTraceOp> op) {
    data.setTrace(op.UNSAFE_get());
    return *this;
  }

  //TODO(abhishek): Taint return value here ?
  JS::CompartmentSpecifier compartmentSpecifier() const { return data.compartmentSpecifier(); }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setNewCompartmentInSystemZone() {
    data.setNewCompartmentInSystemZone();
    return *this;
  }

  //TODO(abhishek): take Tainted<JSObject*> here.
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setNewCompartmentInExistingZone(JSObject* obj) {
    data.setNewCompartmentInExistingZone(obj);
    return *this;
    
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setNewCompartmentAndZone() {
    data.setNewCompartmentAndZone();
    return *this;
  }

  //TODO(abhishek): take Tainted<JSObject*> here.
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setExistingCompartment(JSObject* obj) {
    data.setExistingCompartment(obj);
    return *this;
  }

  //TODO(abhishek): take Tainted<Compartment*> here ?
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setExistingCompartment(JS::Compartment* compartment) {
    data.setExistingCompartment(compartment);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setInvisibleToDebugger(bool flag) {
    data.setInvisibleToDebugger(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setSharedMemoryAndAtomicsEnabled(bool flag) {
    data.setSharedMemoryAndAtomicsEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setDefineSharedArrayBufferConstructor(bool flag) {
    data.setDefineSharedArrayBufferConstructor(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setCoopAndCoepEnabled(bool flag) {
    data.setCoopAndCoepEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setWeakRefsEnabled(JS::WeakRefSpecifier spec) {
    data.setWeakRefsEnabled(spec);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setToSourceEnabled(bool flag) {
    data.setToSourceEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setPropertyErrorMessageFixEnabled(bool flag) {
    data.setPropertyErrorMessageFixEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setIteratorHelpersEnabled(bool flag) {
    data.setIteratorHelpersEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setShadowRealmsEnabled(bool flag) {
    data.setShadowRealmsEnabled(flag);
    return *this;
  }

#ifdef NIGHTLY_BUILD
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setArrayGroupingEnabled(bool flag) {
    data.setArrayGroupingEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setWellFormedUnicodeStringsEnabled(bool flag) {
    data.setWellFormedUnicodeStringsEnabled(flag);
    return *this;
  }
#endif

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setArrayFromAsyncEnabled(bool flag) {
    data.setArrayFromAsyncEnabled(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setChangeArrayByCopyEnabled(bool flag) {
    data.setChangeArrayByCopyEnabled(flag);
    return *this;
  }

#ifdef ENABLE_NEW_SET_METHODS
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setNewSetMethodsEnabled(bool flag) {
    data.setNewSetMethodsEnabled(flag);
    return *this;
  }
#endif
  
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setSecureContext(bool flag) {
    data.setSecureContext(flag);
    return *this;
  }
  
  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setFreezeBuiltins(bool flag) {
    data.setFreezeBuiltins(flag);
    return *this;
  }

  TaintedVolatile<JS::RealmCreationOptions, MC_Sbx>& setProfilerRealmID(uint64_t id) {
    data.setProfilerRealmID(id);
    return *this;
  }
};

template <typename MC_Sbx>
class TaintedVolatile<JS::RealmBehaviors, MC_Sbx> {
private:
  JS::RealmBehaviors data;

public:
  TaintedVolatile<JS::RealmBehaviors, MC_Sbx>& setDiscardSource(bool flag) {
    data.setDiscardSource(flag);
    return *this;
  }
  
  TaintedVolatile<JS::RealmBehaviors, MC_Sbx>& setClampAndJitterTime(bool flag) {
    data.setClampAndJitterTime(flag);
    return *this;
  }
  
  TaintedVolatile<JS::RealmBehaviors, MC_Sbx>& setShouldResistFingerprinting(bool flag) {
    data.setShouldResistFingerprinting(flag);
    return *this;
  }
};

template <typename MC_Sbx>
class TaintedVolatile<JS::RealmOptions, MC_Sbx> {
private:
  JS::RealmOptions data;

public:
  Tainted<JS::RealmCreationOptions*, MC_Sbx> creationOptions() {
    Tainted<JS::RealmCreationOptions*, MC_Sbx> ret(nullptr);
    ret.assign_raw_pointer(&data.creationOptions());
    return ret;
  }

  Tainted<const JS::RealmCreationOptions*, MC_Sbx> creationOptions() const {
    Tainted<const JS::RealmCreationOptions*, MC_Sbx> ret(nullptr);
    ret.assign_raw_pointer(&data.creationOptions());
    return ret;
  }
  
  Tainted<JS::RealmBehaviors*, MC_Sbx> behaviors() {
    Tainted<JS::RealmBehaviors*, MC_Sbx> ret(nullptr);
    ret.assign_raw_pointer(&data.behaviors());
    return ret;
  }

  Tainted<const JS::RealmBehaviors*, MC_Sbx> behaviors() const {
    Tainted<const JS::RealmBehaviors*, MC_Sbx> ret(nullptr);
    ret.assign_raw_pointer(&data.behaviors());
    return ret;
  }

};
 
}
}

#endif

#endif
