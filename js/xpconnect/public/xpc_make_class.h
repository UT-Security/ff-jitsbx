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
#include "js/Class.h"
#include "monkeycage/Sandbox.h"

bool XPC_WN_MaybeResolvingPropertyStub(JSContext* cx, JS::HandleObject obj,
                                       JS::HandleId id, JS::HandleValue v);
extern monkeycage::LazySandboxCallback<JSAddPropertyOp> XPC_WN_MaybeResolvingPropertyStubCb;

bool XPC_WN_CannotModifyPropertyStub(JSContext* cx, JS::HandleObject obj,
                                     JS::HandleId id, JS::HandleValue v);
extern monkeycage::LazySandboxCallback<JSAddPropertyOp> XPC_WN_CannotModifyPropertyStubCb;

bool XPC_WN_MaybeResolvingDeletePropertyStub(JSContext* cx,
                                             JS::HandleObject obj,
                                             JS::HandleId id,
                                             JS::ObjectOpResult& result);
extern monkeycage::LazySandboxCallback<JSDeletePropertyOp> XPC_WN_MaybeResolvingDeletePropertyStubCb;

bool XPC_WN_CannotDeletePropertyStub(JSContext* cx, JS::HandleObject obj,
                                     JS::HandleId id,
                                     JS::ObjectOpResult& result);
extern monkeycage::LazySandboxCallback<JSDeletePropertyOp> XPC_WN_CannotDeletePropertyStubCb;

bool XPC_WN_Shared_Enumerate(JSContext* cx, JS::HandleObject obj);
extern monkeycage::LazySandboxCallback<JSEnumerateOp> XPC_WN_Shared_EnumerateCb;

bool XPC_WN_NewEnumerate(JSContext* cx, JS::HandleObject obj,
                         JS::MutableHandleIdVector properties,
                         bool enumerableOnly);
extern monkeycage::LazySandboxCallback<JSNewEnumerateOp> XPC_WN_NewEnumerateCb;

bool XPC_WN_Helper_Resolve(JSContext* cx, JS::HandleObject obj, JS::HandleId id,
                           bool* resolvedp);
extern monkeycage::LazySandboxCallback<JSResolveOp> XPC_WN_Helper_ResolveCb;

void XPC_WN_Helper_Finalize(JS::GCContext* gcx, JSObject* obj);
extern monkeycage::LazySandboxCallback<JSFinalizeOp> XPC_WN_Helper_FinalizeCb;


void XPC_WN_NoHelper_Finalize(JS::GCContext* gcx, JSObject* obj);
extern monkeycage::LazySandboxCallback<JSFinalizeOp> XPC_WN_NoHelper_FinalizeCb;

bool XPC_WN_Helper_Call(JSContext* cx, unsigned argc, JS::Value* vp);
extern monkeycage::LazySandboxCallback<JSNative> XPC_WN_Helper_CallCb;

bool XPC_WN_Helper_Construct(JSContext* cx, unsigned argc, JS::Value* vp);
extern monkeycage::LazySandboxCallback<JSNative> XPC_WN_Helper_ConstructCb;

void XPCWrappedNative_Trace(JSTracer* trc, JSObject* obj);
extern monkeycage::LazySandboxCallback<JSTraceOp> XPCWrappedNative_TraceCb;

extern const js::ClassExtension* XPC_WN_JSClassExtension();

#define XPC_MAKE_CLASS_OPS(_flags)                                            \
  {                                                                           \
    /* addProperty */                                                         \
    ((_flags)&XPC_SCRIPTABLE_USE_JSSTUB_FOR_ADDPROPERTY) ? nullptr            \
    : ((_flags)&XPC_SCRIPTABLE_ALLOW_PROP_MODS_DURING_RESOLVE)                \
        ? XPC_WN_MaybeResolvingPropertyStubCb.get()                           \
        : XPC_WN_CannotModifyPropertyStubCb.get(),                            \
                                                                              \
        /* delProperty */                                                     \
        ((_flags)&XPC_SCRIPTABLE_USE_JSSTUB_FOR_DELPROPERTY) ? nullptr        \
        : ((_flags)&XPC_SCRIPTABLE_ALLOW_PROP_MODS_DURING_RESOLVE)            \
            ? XPC_WN_MaybeResolvingDeletePropertyStubCb.get()                 \
            : XPC_WN_CannotDeletePropertyStubCb.get(),                        \
                                                                              \
        /* enumerate */                                                       \
        ((_flags)&XPC_SCRIPTABLE_WANT_NEWENUMERATE)                           \
            ? nullptr /* We will use newEnumerate set below in this case */   \
            : XPC_WN_Shared_EnumerateCb.get(),                                \
                                                                              \
        /* newEnumerate */                                                    \
        ((_flags)&XPC_SCRIPTABLE_WANT_NEWENUMERATE) ? XPC_WN_NewEnumerateCb.get()     \
                                                    : nullptr,                \
                                                                              \
        /* resolve */ /* We have to figure out resolve strategy at call time  \
                       */                                                     \
        XPC_WN_Helper_ResolveCb.get(),                                                \
                                                                              \
        /* mayResolve */                                                      \
        nullptr,                                                              \
                                                                              \
        /* finalize */                                                        \
        ((_flags)&XPC_SCRIPTABLE_WANT_FINALIZE) ? XPC_WN_Helper_FinalizeCb.get()      \
                                                : XPC_WN_NoHelper_FinalizeCb.get(),   \
                                                                              \
        /* call */                                                            \
        ((_flags)&XPC_SCRIPTABLE_WANT_CALL) ? XPC_WN_Helper_CallCb.get() : nullptr,   \
                                                                              \
        /* construct */                                                       \
        ((_flags)&XPC_SCRIPTABLE_WANT_CONSTRUCT) ? XPC_WN_Helper_ConstructCb.get()    \
                                                 : nullptr,                   \
                                                                              \
        /* trace */                                                           \
        ((_flags)&XPC_SCRIPTABLE_IS_GLOBAL_OBJECT) ? (JSTraceOp)sbx_addr((void*)JS_GlobalObjectTraceHook) \
                                                   : XPCWrappedNative_TraceCb.get(),  \
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
