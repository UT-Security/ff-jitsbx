/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef xpc_make_class_h
#define xpc_make_class_h

// This file should be used to create JSClass instances for nsIXPCScriptable
// instances. This includes any file that uses xpc_map_end.h.

#include "xpcpublic.h"
#include "mozilla/dom/DOMJSClass.h"
#include "monkeycage/Sandbox.h"

bool XPC_WN_MaybeResolvingPropertyStub(JSContext* cx, JS::HandleObject obj,
                                       JS::HandleId id, JS::HandleValue v);
MC::SandboxCallback<JSAddPropertyOp> XPC_WN_MaybeResolvingPropertyStubCb();

bool XPC_WN_CannotModifyPropertyStub(JSContext* cx, JS::HandleObject obj,
                                     JS::HandleId id, JS::HandleValue v);
MC::SandboxCallback<JSAddPropertyOp> XPC_WN_CannotModifyPropertyStubCb();

bool XPC_WN_MaybeResolvingDeletePropertyStub(JSContext* cx,
                                             JS::HandleObject obj,
                                             JS::HandleId id,
                                             JS::ObjectOpResult& result);
MC::SandboxCallback<JSDeletePropertyOp> XPC_WN_MaybeResolvingDeletePropertyStubCb();

bool XPC_WN_CannotDeletePropertyStub(JSContext* cx, JS::HandleObject obj,
                                     JS::HandleId id,
                                     JS::ObjectOpResult& result);
MC::SandboxCallback<JSDeletePropertyOp> XPC_WN_CannotDeletePropertyStubCb();

bool XPC_WN_Shared_Enumerate(JSContext* cx, JS::HandleObject obj);
MC::SandboxCallback<JSEnumerateOp> XPC_WN_Shared_EnumerateCb();

bool XPC_WN_NewEnumerate(JSContext* cx, JS::HandleObject obj,
                         JS::MutableHandleIdVector properties,
                         bool enumerableOnly);
MC::SandboxCallback<JSNewEnumerateOp> XPC_WN_NewEnumerateCb();

bool XPC_WN_Helper_Resolve(JSContext* cx, JS::HandleObject obj, JS::HandleId id,
                           bool* resolvedp);
MC::SandboxCallback<JSResolveOp> XPC_WN_Helper_ResolveCb();

void XPC_WN_Helper_Finalize(JS::GCContext* gcx, JSObject* obj);
MC::SandboxCallback<JSFinalizeOp> XPC_WN_Helper_FinalizeCb();

void XPC_WN_NoHelper_Finalize(JS::GCContext* gcx, JSObject* obj);
MC::SandboxCallback<JSFinalizeOp> XPC_WN_NoHelper_FinalizeCb();

bool XPC_WN_Helper_Call(JSContext* cx, unsigned argc, JS::Value* vp);
MC::SandboxCallback<JSNative> XPC_WN_Helper_CallCb();

bool XPC_WN_Helper_Construct(JSContext* cx, unsigned argc, JS::Value* vp);
MC::SandboxCallback<JSNative> XPC_WN_Helper_ConstructCb();

void XPCWrappedNative_Trace(JSTracer* trc, JSObject* obj);
MC::SandboxCallback<JSTraceOp> XPCWrappedNative_TraceCb();

extern const js::ClassExtension* XPC_WN_JSClassExtension();

#define XPC_MAKE_CLASS_OPS(_flags)                                            \
  {                                                                           \
    /* addProperty */                                                         \
    ((_flags)&XPC_SCRIPTABLE_USE_JSSTUB_FOR_ADDPROPERTY) ? nullptr            \
    : ((_flags)&XPC_SCRIPTABLE_ALLOW_PROP_MODS_DURING_RESOLVE)                \
        ? XPC_WN_MaybeResolvingPropertyStubCb().UNSAFE_get()                                   \
        : XPC_WN_CannotModifyPropertyStubCb().UNSAFE_get(),                                    \
                                                                              \
        /* delProperty */                                                     \
        ((_flags)&XPC_SCRIPTABLE_USE_JSSTUB_FOR_DELPROPERTY) ? nullptr        \
        : ((_flags)&XPC_SCRIPTABLE_ALLOW_PROP_MODS_DURING_RESOLVE)            \
            ? XPC_WN_MaybeResolvingDeletePropertyStubCb().UNSAFE_get()                         \
            : XPC_WN_CannotDeletePropertyStubCb().UNSAFE_get(),                                \
                                                                              \
        /* enumerate */                                                       \
        ((_flags)&XPC_SCRIPTABLE_WANT_NEWENUMERATE)                           \
            ? nullptr /* We will use newEnumerate set below in this case */   \
            : XPC_WN_Shared_EnumerateCb().UNSAFE_get(),                                        \
                                                                              \
        /* newEnumerate */                                                    \
        ((_flags)&XPC_SCRIPTABLE_WANT_NEWENUMERATE) ? XPC_WN_NewEnumerateCb().UNSAFE_get()     \
                                                    : nullptr,                \
                                                                              \
        /* resolve */ /* We have to figure out resolve strategy at call time  \
                       */                                                     \
        XPC_WN_Helper_ResolveCb().UNSAFE_get(),                                                \
                                                                              \
        /* mayResolve */                                                      \
        nullptr,                                                              \
                                                                              \
        /* finalize */                                                        \
        ((_flags)&XPC_SCRIPTABLE_WANT_FINALIZE) ? XPC_WN_Helper_FinalizeCb().UNSAFE_get()      \
                                                : XPC_WN_NoHelper_FinalizeCb().UNSAFE_get(),   \
                                                                              \
        /* call */                                                            \
        ((_flags)&XPC_SCRIPTABLE_WANT_CALL) ? XPC_WN_Helper_CallCb().UNSAFE_get() : nullptr,   \
                                                                              \
        /* construct */                                                       \
        ((_flags)&XPC_SCRIPTABLE_WANT_CONSTRUCT) ? XPC_WN_Helper_ConstructCb().UNSAFE_get()    \
                                                 : nullptr,                   \
                                                                              \
        /* trace */                                                           \
        ((_flags)&XPC_SCRIPTABLE_IS_GLOBAL_OBJECT) ? MC::Sandbox::Address(JS_GlobalObjectTraceHook) \
                                                   : XPCWrappedNative_TraceCb().UNSAFE_get(),  \
  }

#define XPC_MAKE_CLASS(_name, _flags, _classOps)                   \
  {                                                                \
    /* name */                                                     \
    _name,                                                         \
                                                                   \
        /* flags */                                                \
        JSCLASS_SLOT0_IS_NSISUPPORTS | JSCLASS_IS_WRAPPED_NATIVE | \
            JSCLASS_FOREGROUND_FINALIZE |                          \
            (((_flags)&XPC_SCRIPTABLE_IS_GLOBAL_OBJECT)            \
                 ? XPCONNECT_GLOBAL_FLAGS                          \
                 : JSCLASS_HAS_RESERVED_SLOTS(1)),                 \
                                                                   \
        /* cOps */                                                 \
        _classOps,                                                 \
                                                                   \
        /* spec */                                                 \
        nullptr,                                                   \
                                                                   \
        /* ext */                                                  \
        XPC_WN_JSClassExtension(),                                 \
                                                                   \
        /* oOps */                                                 \
        nullptr,                                                   \
  }

#endif
