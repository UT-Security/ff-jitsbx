/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Telemetry and use counter functionality. */

#ifndef mc_friend_UsageStatistics_h
#define mc_friend_UsageStatistics_h

#include "js/friend/UsageStatistics.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

inline void JS_SetAccumulateTelemetryCallback(
    MCContext* cx, MC::SandboxCallback<JSAccumulateTelemetryDataCallback> callback) {
  return JS_SetAccumulateTelemetryCallback(cx->cx_, callback.UNSAFE_get());
}

inline void JS_SetSetUseCounterCallback(
    MCContext* cx, MC::SandboxCallback<JSSetUseCounterCallback> callback) {
  return JS_SetSetUseCounterCallback(cx->cx_, callback.UNSAFE_get());
}

#endif

#endif
