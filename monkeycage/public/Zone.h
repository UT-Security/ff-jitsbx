/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript API. */

#ifndef mc_Zone_h
#define mc_Zone_h

#include "js/Zone.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

inline void JS_SetDestroyZoneCallback(
    MCContext* cx, MC::Sandbox::Callback<JSDestroyZoneCallback> callback) {
  JS_SetDestroyZoneCallback(cx->cx_, callback.UNSAFE_get());  
}

inline void JS_SetDestroyCompartmentCallback(
    MCContext* cx, MC::Sandbox::Callback<JSDestroyCompartmentCallback> callback) {
    return JS_SetDestroyCompartmentCallback(cx->cx_, callback.UNSAFE_get());
}

inline void JS_SetSizeOfIncludingThisCompartmentCallback(
    MCContext* cx, MC::Sandbox::Callback<JSSizeOfIncludingThisCompartmentCallback> callback) {
    return JS_SetSizeOfIncludingThisCompartmentCallback(cx->cx_, callback.UNSAFE_get());
}

inline bool JS_RefreshCrossCompartmentWrappers(MCContext* cx,
                                               JS::Handle<JSObject*> obj) {
    return JS_RefreshCrossCompartmentWrappers(cx->cx_, obj);
}

inline void JS_MarkCrossZoneId(MCContext* cx, jsid id) {
    return JS_MarkCrossZoneId(cx->cx_, id);
}

inline void JS_MarkCrossZoneIdValue(MCContext* cx, const JS::Value& value) {
    return JS_MarkCrossZoneIdValue(cx->cx_, value);
}
#endif

#endif
