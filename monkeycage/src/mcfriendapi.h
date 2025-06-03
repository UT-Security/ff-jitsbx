/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mcfriendapi_h
#define mcfriendapi_h

#include "jsfriendapi.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Principals.h"
#include "monkeycage/Sandbox.h"

namespace js {
inline JSFunction* DefineFunctionWithReserved(MCContext* cx, JSObject* obj,
                                       const char* name,
                                       MC::SandboxCallback<JSNative> call,
                                       unsigned nargs, unsigned attrs) {
  return DefineFunctionWithReserved(cx->cx_, obj, name, call.UNSAFE_get(),
                                    nargs, attrs);
}

inline JSFunction* NewFunctionWithReserved(MCContext* cx,
                                    MC::SandboxCallback<JSNative> call,
                                    unsigned nargs, unsigned flags,
                                    const char* name) {
  return NewFunctionWithReserved(cx->cx_, call.UNSAFE_get(), nargs, flags,
                                 name);
}

inline JSFunction* NewFunctionByIdWithReserved(MCContext* cx,
                                        MC::SandboxCallback<JSNative> native,
                                        unsigned nargs, unsigned flags,
                                        jsid id) {
  return NewFunctionByIdWithReserved(cx->cx_, native.UNSAFE_get(), nargs, flags,
                                     id);
}
}  // namespace js

namespace mc {

struct JSDOMCallbacks {
private:
  js::JSDOMCallbacks inner_;
public:
 explicit JSDOMCallbacks(
     MC::Sandbox::Callback<js::DOMInstanceClassHasProtoAtDepth>
         instanceClassMatchesProto)
     : inner_{instanceClassMatchesProto.UNSAFE_get()} {}
 const js::JSDOMCallbacks* UNSAFE_get() const { return &inner_; }
};

using DOMCallbacks = struct JSDOMCallbacks;

}

namespace JS {

inline void SetRealmPrincipals(JS::Realm* realm, MCPrincipals* principals) {
  return SetRealmPrincipals(realm, principals->inner_);
}
}

namespace js {

inline bool AreGCGrayBitsValid(MCRuntime* rt) {
    return AreGCGrayBitsValid(rt->rt_);
}

#if defined(JS_GC_ZEAL) || defined(DEBUG)
inline bool CheckGrayMarkingState(MCRuntime* rt) {
    return CheckGrayMarkingState(rt->rt_);
}
#endif

inline void SetDOMCallbacks(MCContext* cx, const mc::DOMCallbacks* callbacks) {
  return SetDOMCallbacks(cx->cx_, callbacks->UNSAFE_get());
}

inline const DOMCallbacks* GetDOMCallbacks(MCContext* cx) {
  return GetDOMCallbacks(cx->cx_);
}

inline JSLinearString* GetErrorTypeName(MCContext* cx,
                                                      int16_t exnType) {
  return GetErrorTypeName(cx->cx_, exnType);
}

//TODO(abhishek): ScriptEnvironmentPreparer has virtual methods, so
// we can't directly accept a pointer to a class implemented in Gecko.
inline void SetScriptEnvironmentPreparer(
    MCContext* cx, ScriptEnvironmentPreparer* preparer) {
  return SetScriptEnvironmentPreparer(cx->cx_, preparer);
}

inline void SetPreserveWrapperCallbacks(
    MCContext* cx, MC::Sandbox::Callback<PreserveWrapperCallback> preserveWrapper,
    MC::Sandbox::Callback<HasReleasedWrapperCallback> hasReleasedWrapper) {
  return SetPreserveWrapperCallbacks(cx->cx_, preserveWrapper.UNSAFE_get(), hasReleasedWrapper.UNSAFE_get());
}

inline bool IsObjectInContextCompartment(JSObject* obj, const MCContext* cx) {
  return IsObjectInContextCompartment(obj, cx->cx_);
}
}  // namespace js

inline void JS_SetGrayGCRootsTracer(
    MCContext* cx, MC::Sandbox::Callback<JSGrayRootsTracer> traceOp,
    void* data) {
  return JS_SetGrayGCRootsTracer(cx->cx_, traceOp.UNSAFE_get(), data);
}

namespace mc {
class CompartmentTransplantCallback {
 public:
  js::CompartmentTransplantCallback* inner_;
 private:

  static JSObject* getObjectToTransplantCb(void* p, JS::Compartment* compartment) {
    auto outer = static_cast<CompartmentTransplantCallback*>(p);      
    return outer->getObjectToTransplant(compartment);
  }

  MC::SandboxCallback<js::sandbox::CompartmentTransplantCallback::GetObjectToTransplantOp> op() {
    static auto inner_ = MC::Sandbox::RegisterCallback(getObjectToTransplantCb);
    return inner_;
  }
 public:
  CompartmentTransplantCallback() {
    inner_ = js_new<js::sandbox::CompartmentTransplantCallback>(op().UNSAFE_get(), this);  
  }

  ~CompartmentTransplantCallback() {
    js_free((void*)inner_);
  }
  
  virtual JSObject* getObjectToTransplant(JS::Compartment* compartment) = 0;
};

class CompartmentFilter {
 public:
  js::CompartmentFilter* inner_;

 private:
  static bool matchCb(const void* p, JS::Compartment* c) {
    auto filter = static_cast<const CompartmentFilter*>(p);
    return filter->match(c);
  }

  static MC::SandboxCallback<js::sandbox::CompartmentFilter::MatchOp> op() {
    static auto inner_ = MC::Sandbox::RegisterCallback(matchCb);
    return inner_;
  }

 public:
  CompartmentFilter() {
    inner_ = js_new<js::sandbox::CompartmentFilter>(op().UNSAFE_get(), this);
  }

  ~CompartmentFilter() { js_free((void*)inner_); }

  virtual bool match(JS::Compartment* c) const = 0;
};

struct AllCompartments : public CompartmentFilter {
  virtual bool match(JS::Compartment* c) const override { return true; }
};

struct SingleCompartment : public CompartmentFilter {
  JS::Compartment* ours;
  explicit SingleCompartment(JS::Compartment* c) : ours(c) {}
  virtual bool match(JS::Compartment* c) const override { return c == ours; }
};
}  // namespace mc

namespace js {

inline void RemapRemoteWindowProxies(
    JSContext* cx, mc::CompartmentTransplantCallback* callback,
    JS::MutableHandleObject newTarget) {
  return RemapRemoteWindowProxies(cx, callback->inner_, newTarget);  
}

inline bool NukeCrossCompartmentWrappers(
    JSContext* cx, const mc::CompartmentFilter& sourceFilter, JS::Realm* target,
    NukeReferencesToWindow nukeReferencesToWindow,
    NukeReferencesFromTarget nukeReferencesFromTarget) {
  return NukeCrossCompartmentWrappers(cx, *sourceFilter.inner_, target,
                                      nukeReferencesToWindow,
                                      nukeReferencesFromTarget);
}
}  // namespace js

namespace mc {

struct WeakMapTracer {
  js::WeakMapTracer* inner_;

 private:
  static void traceCb(void* p, JSObject* m, JS::GCCellPtr key,
                      JS::GCCellPtr value) {
    auto tracer = static_cast<mc::WeakMapTracer*>(p);
    return tracer->trace(m, key, value);
  }

  static MC::SandboxCallback<js::sandbox::WeakMapTracer::TraceOp> op() {
    static auto inner_ = MC::Sandbox::RegisterCallback(traceCb);
    return inner_;
  }

 public:
  explicit WeakMapTracer(JSRuntime* rt) {
    inner_ = js_new<js::sandbox::WeakMapTracer>(op().UNSAFE_get(), this, rt);
  }

  ~WeakMapTracer() { js_free((void*)inner_); }

  virtual void trace(JSObject* m, JS::GCCellPtr key, JS::GCCellPtr value) = 0;
};

}  // namespace mc

namespace js {
  
inline void TraceWeakMaps(mc::WeakMapTracer* trc) {
  return TraceWeakMaps(trc->inner_);
}

}

#else

namespace mc {
using DOMCallbacks = js::DOMCallbacks;
using CompartmentTransplantCallback = js::CompartmentTransplantCallback;

using CompartmentFilter = js::CompartmentFilter;
using AllCompartments = js::AllCompartments;
using SingleCompartment = js::SingleCompartment;

using WeakMapTracer = js::WeakMapTracer;
}

#endif

#endif
