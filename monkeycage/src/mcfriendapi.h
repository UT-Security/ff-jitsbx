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
#include "monkeycage/Sandbox.h"

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

namespace js {

inline bool AreGCGrayBitsValid(MCRuntime* rt) {
    return AreGCGrayBitsValid(rt->rt_);
}

inline bool CheckGrayMarkingState(MCRuntime* rt) {
    return CheckGrayMarkingState(rt->rt_);
}

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
}

inline void JS_SetGrayGCRootsTracer(
    MCContext* cx, MC::Sandbox::Callback<JSGrayRootsTracer> traceOp,
    void* data) {
  return JS_SetGrayGCRootsTracer(cx->cx_, traceOp.UNSAFE_get(), data);
}

#else

namespace mc {
using DOMCallbacks = js::DOMCallbacks;
}

#endif

#endif
