/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript API. */

#ifndef mcapi_h
#define mcapi_h

#include "jsapi.h"

#ifdef JS_SANDBOX

#include "monkeycage/Class.h"
#include "monkeycage/Context.h"
#include "monkeycage/Debug.h"
#include "monkeycage/ErrorReport.h"
#include "monkeycage/Exception.h"
#include "monkeycage/GCAPI.h"
#include "monkeycage/GCVector.h"
#include "monkeycage/GlobalObject.h"
#include "monkeycage/Id.h"
#include "monkeycage/Interrupt.h"
#include "monkeycage/MemoryCallbacks.h"
#include "monkeycage/PropertyAndElement.h"  // JS_Enumerate
#include "monkeycage/Realm.h"
#include "monkeycage/RealmIterators.h"
#include "monkeycage/RealmOptions.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/ScriptPrivate.h"
#include "monkeycage/Stack.h"
#include "monkeycage/StreamConsumer.h"
#include "monkeycage/String.h"
#include "monkeycage/TracingAPI.h"
#include "monkeycage/TypeDecls.h"
#include "monkeycage/UniquePtr.h"
#include "monkeycage/Utility.h"
#include "monkeycage/Value.h"
#include "monkeycage/ValueArray.h"
#include "monkeycage/WaitCallbacks.h"
#include "monkeycage/WrapperCallbacks.h"

#include "monkeycage/Sandbox.h"
#include "monkeycage/SandboxHeap.h"
#include "monkeycage/SandboxStack.h"
#include "monkeycage/Tainted.h"

inline bool JS_ValueToObject(MCContext* cx, JS::HandleValue v,
                             JS::MutableHandleObject objp) {
  return JS_ValueToObject(cx->cx_, v, objp);
}

inline JSFunction* JS_ValueToFunction(MCContext* cx, JS::HandleValue v) {
  return JS_ValueToFunction(cx->cx_, v);
}

inline JSFunction* JS_ValueToConstructor(MCContext* cx, JS::HandleValue v) {
  return JS_ValueToConstructor(cx->cx_, v);
}

inline JSString* JS_ValueToSource(MCContext* cx, JS::Handle<JS::Value> v) {
  return JS_ValueToSource(cx->cx_, v);
}

inline JSType JS_TypeOfValue(MCContext* cx, JS::Handle<JS::Value> v) {
  return JS_TypeOfValue(cx->cx_, v);  
}

inline void JS_SetWrapObjectCallbacks(MCContext* cx,
                                      const MCWrapObjectCallbacks* callbacks) {
  JS_SetWrapObjectCallbacks(cx->cx_, callbacks->UNSAFE_get());
}

inline bool JS_WrapObject(MCContext* cx, JS::MutableHandleObject objp) {
  return JS_WrapObject(cx->cx_, objp);
}

inline bool JS_WrapValue(MCContext* cx, JS::MutableHandleValue vp) {
  return JS_WrapValue(cx->cx_, vp);
}

inline JSObject* JS_TransplantObject(MCContext* cx, JS::HandleObject origobj,
                                     JS::HandleObject target) {
  return JS_TransplantObject(cx->cx_, origobj, target);
}

inline bool JS_ResolveStandardClass(MCContext* cx, JS::HandleObject obj,
                                    JS::HandleId id, MC::Tainted<bool*> resolved) {
  return JS_ResolveStandardClass(cx->cx_, obj, id, resolved.UNSAFE_unverified());
}

inline bool JS_EnumerateStandardClasses(MCContext* cx, JS::HandleObject obj) {
  return JS_EnumerateStandardClasses(cx->cx_, obj);
}

inline bool JS_NewEnumerateStandardClasses(MCContext* cx, JS::HandleObject obj,
                                           JS::MutableHandleIdVector properties,
                                           bool enumerableOnly) {
  return JS_NewEnumerateStandardClasses(cx->cx_, obj, properties,
                                        enumerableOnly);
}

inline bool JS_NewEnumerateStandardClassesIncludingResolved(
    MCContext* cx, JS::HandleObject obj, JS::MutableHandleIdVector properties,
    bool enumerableOnly) {
  return JS_NewEnumerateStandardClassesIncludingResolved(
      cx->cx_, obj, properties, enumerableOnly);
}

inline bool JS_GetClassObject(MCContext* cx, JSProtoKey key,
                              JS::MutableHandle<JSObject*> objp) {
  return JS_GetClassObject(cx->cx_, key, objp);
}

inline bool JS_GetClassPrototype(MCContext* cx, JSProtoKey key,
                                 JS::MutableHandle<JSObject*> objp) {
  return JS_GetClassPrototype(cx->cx_, key, objp);
}

inline JSProtoKey JS_IdToProtoKey(MCContext* cx, JS::HandleId id) {
  return JS_IdToProtoKey(cx->cx_, id);
}

inline bool JS_ValueToId(MCContext* cx, JS::HandleValue v,
                         JS::MutableHandleId idp) {
  return JS_ValueToId(cx->cx_, v, idp);
}

inline bool JS_StringToId(MCContext* cx, JS::HandleString s,
                          JS::MutableHandleId idp) {
  return JS_StringToId(cx->cx_, s, idp);
}

inline bool JS_IdToValue(MCContext* cx, jsid id,
                         JS::MutableHandle<JS::Value> vp) {
  return JS_IdToValue(cx->cx_, id, vp);
}

namespace JS {

inline bool ToPrimitive(MCContext* cx, JS::HandleObject obj, JSType hint,
                        JS::MutableHandleValue vp) {
  return ToPrimitive(cx->cx_, obj, hint, vp);
}

inline bool GetFirstArgumentAsTypeHint(MCContext* cx, CallArgs args,
                                       MC::Tainted<JSType*> result) {
  return GetFirstArgumentAsTypeHint(cx->cx_, args,
                                    result.INTERNAL_unverified_safe());
}
}  // namespace JS

inline bool JS_LinkConstructorAndPrototype(MCContext* cx,
                                           JS::Handle<JSObject*> ctor,
                                           JS::Handle<JSObject*> proto) {
  return JS_LinkConstructorAndPrototype(cx->cx_, ctor, proto);
}

inline bool JS_InstanceOf(MCContext* cx, JS::Handle<JSObject*> obj,
                          const JSClass* clasp, JS::CallArgs* args) {
  return JS_InstanceOf(cx->cx_, obj, clasp, args);
}

inline bool JS_HasInstance(MCContext* cx, JS::Handle<JSObject*> obj,
                           JS::Handle<JS::Value> v, bool* bp) {
  return JS_HasInstance(cx->cx_, obj, v, bp);
}

inline JSObject* JS_GetConstructor(MCContext* cx, JS::Handle<JSObject*> proto) {
  return JS_GetConstructor(cx->cx_, proto);
}

inline JSObject* JS_NewObject(MCContext* cx, const JSClass* clasp) {
  return JS_NewObject(cx->cx_, clasp);
}

inline JSObject* JS_NewObjectWithGivenProto(MCContext* cx, const JSClass* clasp,
                                     JS::Handle<JSObject*> proto) {
  return JS_NewObjectWithGivenProto(cx->cx_, clasp, proto);
}

inline JSObject* JS_NewPlainObject(MCContext* cx) {
  return JS_NewPlainObject(cx->cx_);
}

inline bool JS_GetPrototype(MCContext* cx, JS::HandleObject obj,
                            JS::MutableHandleObject result) {
  return JS_GetPrototype(cx->cx_, obj, result);
}

inline bool JS_GetPrototypeIfOrdinary(MCContext* cx, JS::HandleObject obj,
                                      bool* isOrdinary,
                                      JS::MutableHandleObject result) {
  return JS_GetPrototypeIfOrdinary(cx->cx_, obj, isOrdinary, result);
}

inline bool JS_SetPrototype(MCContext* cx, JS::HandleObject obj,
                            JS::HandleObject proto) {
  return JS_SetPrototype(cx->cx_, obj, proto);
}

inline bool JS_IsExtensible(MCContext* cx, JS::HandleObject obj,
                            MC::Tainted<bool*> extensible) {
  return JS_IsExtensible(cx->cx_, obj, extensible.INTERNAL_unverified_safe());
}

inline bool JS_PreventExtensions(MCContext* cx, JS::HandleObject obj,
                                 MC::Tainted<JS::ObjectOpResult*> result) {
  return JS_PreventExtensions(cx->cx_, obj, *result.INTERNAL_unverified_safe());
}

inline bool JS_SetImmutablePrototype(MCContext* cx, JS::HandleObject obj,
                                     MC::Tainted<bool*> succeeded) {
  return JS_SetImmutablePrototype(cx->cx_, obj,
                                  succeeded.INTERNAL_unverified_safe());
}

inline bool JS_AssignObject(MCContext* cx, JS::HandleObject target,
                            JS::HandleObject src) {
  return JS_AssignObject(cx->cx_, target, src);
}

inline JSFunction* JS_NewFunction(MCContext* cx,
                                  MC::SandboxCallback<JSNative> call,
                                  unsigned nargs, unsigned flags,
                                  const char* name) {
  return JS_NewFunction(cx->cx_, call.UNSAFE_get(), nargs, flags, name);
}

namespace JS {

inline JSFunction* GetSelfHostedFunction(MCContext* cx,
                                         const char* selfHostedName,
                                         HandleId id, unsigned nargs) {
  return GetSelfHostedFunction(cx->cx_, selfHostedName, id, nargs);
}

inline JSFunction* NewFunctionFromSpec(MCContext* cx, const JSFunctionSpec* fs,
                                       HandleId id) {
  return NewFunctionFromSpec(cx->cx_, fs, id);
}

inline JSFunction* NewFunctionFromSpec(MCContext* cx, const JSFunctionSpec* fs) {
  return NewFunctionFromSpec(cx->cx_, fs);
}
}  // namespace JS

inline bool JS_GetFunctionLength(MCContext* cx, JS::HandleFunction fun,
                                 uint16_t* length) {
  return JS_GetFunctionLength(cx->cx_, fun, length);
}

inline unsigned JS_GetScriptBaseLineNumber(MCContext* cx, JSScript* script) {
  return JS_GetScriptBaseLineNumber(cx->cx_, script);
}

inline JSScript* JS_GetFunctionScript(MCContext* cx, JS::HandleFunction fun) {
  return JS_GetFunctionScript(cx->cx_, fun);
}

inline JSString* JS_DecompileScript(MCContext* cx, JS::Handle<JSScript*> script) {
  return JS_DecompileScript(cx->cx_, script);
}

inline  JSString* JS_DecompileFunction(MCContext* cx, JS::Handle<JSFunction*> fun) {
  return JS_DecompileFunction(cx->cx_, fun);
}

namespace JS {

inline bool PropertySpecNameToPermanentId(MCContext* cx,
                                          JSPropertySpec::Name name,
                                          jsid* idp) {
  return PropertySpecNameToPermanentId(cx->cx_, name, idp);
}
} /* namespace JS */

inline JSObject* JS_NewObjectForConstructor(MCContext* cx, const JSClass* clasp,
                                            const JS::CallArgs& args) {
  return JS_NewObjectForConstructor(cx->cx_, clasp, args);
}

inline void JS_SetParallelParsingEnabled(MCContext* cx, bool enabled) {
  return JS_SetParallelParsingEnabled(cx->cx_, enabled);
}

inline void JS_SetOffthreadIonCompilationEnabled(MCContext* cx,
                                                               bool enabled) {
  return JS_SetOffthreadIonCompilationEnabled(cx->cx_, enabled);
}

inline void JS_SetGlobalJitCompilerOption(MCContext* cx,
                                          JSJitCompilerOption opt,
                                          uint32_t value) {
  return JS_SetGlobalJitCompilerOption(cx->cx_, opt, value);  
}

inline bool JS_GetGlobalJitCompilerOption(MCContext* cx,
                                          JSJitCompilerOption opt,
                                          uint32_t* valueOut) {
  return JS_GetGlobalJitCompilerOption(cx->cx_, opt, valueOut);
}

inline bool JS_IndexToId(MCContext* cx, uint32_t index,
                         JS::MutableHandleId id) {
  return JS_IndexToId(cx->cx_, index, id);
}

inline bool JS_CharsToId(MCContext* cx, JS::TwoByteChars chars,
                         JS::MutableHandleId id) {
  return JS_CharsToId(cx->cx_, chars, id);
}
inline bool JS_IsIdentifier(MCContext* cx, JS::HandleString str,
                            bool* isIdentifier) {
  return JS_IsIdentifier(cx->cx_, str, isIdentifier);
}

namespace JS {

inline bool DescribeScriptedCaller(
    MCContext* cx, AutoFilename* filename = nullptr, unsigned* lineno = nullptr,
    unsigned* column = nullptr) {
  return DescribeScriptedCaller(cx->cx_, filename, lineno, column);
}

inline JSObject* GetScriptedCallerGlobal(MCContext* cx) {
  return GetScriptedCallerGlobal(cx->cx_);
}

inline void HideScriptedCaller(MCContext* cx) {
  return HideScriptedCaller(cx->cx_);
}

inline void UnhideScriptedCaller(MCContext* cx) {
  return UnhideScriptedCaller(cx->cx_);
}
}
#endif

#endif
