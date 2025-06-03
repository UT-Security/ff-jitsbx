/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/sandbox/TracingAPI.h"

using namespace JS::sandbox;

CallbackTracer::CallbackTracer(OnChildOp onChild, void* callbackTracer,
                               JSRuntime* rt, JS::TracerKind kind,
                               JS::TraceOptions options)
    : JS::CallbackTracer(rt, kind, options),
      onChild_(onChild),
      callbackTracer_(callbackTracer) {}

CallbackTracer::CallbackTracer(OnChildOp onChild, void* callbackTracer,
                               JSContext* cx, JS::TracerKind kind,
                               JS::TraceOptions options)
    : JS::CallbackTracer(cx, kind, options),
      onChild_(onChild),
      callbackTracer_(callbackTracer) {}

void CallbackTracer::onChild(JS::GCCellPtr thing, const char* name) {
  onChild_(callbackTracer_, thing, name);
}
