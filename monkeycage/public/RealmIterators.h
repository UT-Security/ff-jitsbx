/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Various interfaces to iterate over the Realms given various context such as
 * principals, compartments and GC zones.
 */

#ifndef mc_RealmIterators_h
#define mc_RealmIterators_h

#include "js/RealmIterators.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Realm.h"
#include "monkeycage/SandboxCallback.h"

namespace JS {

inline void IterateRealms(
    MCContext* cx, void* data,
    MC::SandboxCallback<IterateRealmCallback> realmCallback) {
  return IterateRealms(cx->cx_, data, realmCallback.UNSAFE_get());
}

inline void IterateRealmsWithPrincipals(
    MCContext* cx, JSPrincipals* principals, void* data,
    MC::SandboxCallback<IterateRealmCallback> realmCallback) {
  return IterateRealmsWithPrincipals(cx->cx_, principals, data,
                                     realmCallback.UNSAFE_get());
}

inline void IterateRealmsInCompartment(
    MCContext* cx, JS::Compartment* compartment, void* data,
    MC::SandboxCallback<IterateRealmCallback> realmCallback) {
  return IterateRealmsInCompartment(cx->cx_, compartment, data,
                                    realmCallback.UNSAFE_get());
}

inline void JS_IterateCompartments(
    MCContext* cx, void* data,
    MC::SandboxCallback<JSIterateCompartmentCallback> compartmentCallback) {
  return JS_IterateCompartments(cx->cx_, data,
                                compartmentCallback.UNSAFE_get());
}

inline void JS_IterateCompartmentsInZone(
    MCContext* cx, JS::Zone* zone, void* data,
    MC::SandboxCallback<JSIterateCompartmentCallback> compartmentCallback) {
  return JS_IterateCompartmentsInZone(cx->cx_, zone, data,
                                      compartmentCallback.UNSAFE_get());
}
}

#endif

#endif

