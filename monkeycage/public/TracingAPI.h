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
#include "monkeycage/Sandbox.h"
#include "monkeycage/Tainted.h"
#include "monkeycage/Utility.h"

namespace MC {

class CallbackTracer {
 private:
  MC::Tainted<JS::CallbackTracer*> inner_;

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
    auto inner = mc_new<JS::sandbox::CallbackTracer>(op().UNSAFE_get(), this, rt,
                                                 kind, options);
    inner_.assign_raw_pointer(
        static_cast<JS::CallbackTracer*>(inner.INTERNAL_unverified_safe()));
  }
  CallbackTracer(JSContext* cx, JS::TracerKind kind = JS::TracerKind::Callback,
                 JS::TraceOptions options = JS::TraceOptions()) {
    auto inner = mc_new<JS::sandbox::CallbackTracer>(op().UNSAFE_get(), this, cx,
                                                 kind, options);
    inner_.assign_raw_pointer(
        static_cast<JS::CallbackTracer*>(inner.INTERNAL_unverified_safe()));
  }

  ~CallbackTracer() {
    mc_free(inner_);
  }

  virtual void onChild(JS::GCCellPtr thing, const char* name) = 0;

  operator MC::Tainted<JS::CallbackTracer*>() { return inner_; }
  
  operator MC::Tainted<JSTracer*>() {
    MC::Tainted<JSTracer*> ret;
    ret.assign_raw_pointer(inner_.INTERNAL_unverified_safe());
    return ret;
  }

  MC::Tainted<JS::CallbackTracer*> getCallbackTracer() { return inner_; }

  JS::TracingContext& context() { return inner_.UNSAFE_unverified()->context(); }
};

}

namespace JS {

template <typename T>
inline void TraceEdge(MC::Tainted<JSTracer*> trc, JS::Heap<T>* thingp,
                      const char* name) {
  return TraceEdge(trc.INTERNAL_unverified_safe(), thingp, name);
}

template <typename T>
inline void TraceEdge(MC::Tainted<JSTracer*> trc, JS::TenuredHeap<T>* thingp,
                      const char* name) {
  return TraceEdge(trc.INTERNAL_unverified_safe(), thingp, name);
}

inline void TraceChildren(MC::Tainted<JSTracer*> trc, GCCellPtr thing) {
  return TraceChildren(trc.INTERNAL_unverified_safe(), thing);
}

}  // namespace JS

#else

namespace MC {
using CallbackTracer = JS::CallbackTracer;
}

#endif

#endif
