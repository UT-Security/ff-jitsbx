/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_TracingAPI_h
#define js_sandbox_TracingAPI_h

#include "js/TracingAPI.h"
#include "js/sandbox/RootingAPI.h"

namespace JS {
namespace sandbox {

class JS_PUBLIC_API CallbackTracer : public JS::CallbackTracer {
 public:
  using OnChildOp = void (*)(void* tracer, JS::GCCellPtr thing,
                             const char* name);

 private:
  OnChildOp onChild_;
  void* callbackTracer_;

 public:
  CallbackTracer(OnChildOp onChild, void* callbackTracer, JSRuntime* rt,
                 JS::TracerKind kind = JS::TracerKind::Callback,
                 JS::TraceOptions options = JS::TraceOptions());

  CallbackTracer(OnChildOp onChild, void* callbackTracer, JSContext* cx,
                 JS::TracerKind kind = JS::TracerKind::Callback,
                 JS::TraceOptions options = JS::TraceOptions());

  inline void* getCallbackTracer() { return callbackTracer_; }

  void onChild(JS::GCCellPtr thing, const char* name) override;
};
}  // namespace sandbox

template <typename T>
inline void TraceSecureEdge(JSTracer* trc, T* thingp,
                            const char* name) {

  js::gc::TraceExternalEdge(trc, thingp, name);
}

}  // namespace JS

#endif
