/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_TracingAPI_h
#define mc_TracingAPI_h

#include "js/TracingAPI.h"

#ifdef JS_SANDBOX

#include "js/sandbox/TracingAPI.h"
#include "js/Utility.h"
#include "monkeycage/Sandbox.h"

namespace MC {

class CallbackTracer {
 private:
  JS::CallbackTracer* inner_;

  static void onChildCb(void* p, JS::GCCellPtr thing, const char* name) {
    auto tracer = static_cast<CallbackTracer*>(p);
    tracer->onChild(thing, name);
  }

  static MC::SandboxCallback<JS::sandbox::CallbackTracer::OnChildOp> op() {
    static auto inner_ = MC::Sandbox::RegisterCallback(onChildCb);
    return inner_;
  }

 public:
  CallbackTracer(JSRuntime* rt, JS::TracerKind kind = JS::TracerKind::Callback,
                 JS::TraceOptions options = JS::TraceOptions()) {
    inner_ = js_new<JS::sandbox::CallbackTracer>(op().UNSAFE_get(), this, rt,
                                                 kind, options);
  }
  CallbackTracer(JSContext* cx, JS::TracerKind kind = JS::TracerKind::Callback,
                 JS::TraceOptions options = JS::TraceOptions()) {
    inner_ = js_new<JS::sandbox::CallbackTracer>(op().UNSAFE_get(), this, cx,
                                                 kind, options);
  }

  ~CallbackTracer() {
    js_free((void*)inner_);
  }

  virtual void onChild(JS::GCCellPtr thing, const char* name) = 0;

  inline JS::CallbackTracer* getCallbackTracer() { return inner_; }
};
}

#else

namespace MC {
using CallbackTracer = JS::CallbackTracer;
}

#endif

#endif
