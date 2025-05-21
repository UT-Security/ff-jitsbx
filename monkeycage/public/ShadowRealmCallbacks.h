/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_ShadowReamCallbacks_h
#define mc_ShadowReamCallbacks_h

#include "js/ShadowRealmCallbacks.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"

namespace JS {


inline void SetShadowRealmInitializeGlobalCallback(
    MCContext* cx, MC::Sandbox::Callback<GlobalInitializeCallback> callback) {
  return SetShadowRealmInitializeGlobalCallback(cx->cx_, callback.UNSAFE_get());
}

inline void SetShadowRealmGlobalCreationCallback(
    MCContext* cx, MC::Sandbox::Callback<GlobalCreationCallback> callback) {
  return SetShadowRealmGlobalCreationCallback(cx->cx_, callback.UNSAFE_get());
}
}

#endif

#endif
