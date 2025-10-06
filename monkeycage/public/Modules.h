/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript module (as in, the syntactic construct) operations. */

#ifndef mc_Modules_h
#define mc_Modules_h

#include "js/Modules.h"

#ifdef JS_SANDBOX

#include "monkeycage/CompileOptions.h"
#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

namespace JS {

inline void SetSupportedImportAssertions(
    MCRuntime* rt, const ImportAssertionVector& assertions) {
  return SetSupportedImportAssertions(rt->rt_, assertions);
}

inline ModuleResolveHook GetModuleResolveHook(MCRuntime* rt) { return GetModuleResolveHook(rt->rt_); }

inline void SetModuleResolveHook(MCRuntime* rt,
                                 MC::SandboxCallback<ModuleResolveHook> func) {
  return SetModuleResolveHook(rt->rt_, func.UNSAFE_get());
}

inline ModuleMetadataHook GetModuleMetadataHook(MCRuntime* rt) {
  return GetModuleMetadataHook(rt->rt_);
}

inline void SetModuleMetadataHook(MCRuntime* rt,
                                  MC::SandboxCallback<ModuleMetadataHook> func) {
  return SetModuleMetadataHook(rt->rt_, func.UNSAFE_get());
}

inline ModuleDynamicImportHook GetModuleDynamicImportHook(MCRuntime* rt) {
  return GetModuleDynamicImportHook(rt->rt_);
}

inline void SetModuleDynamicImportHook(
    MCRuntime* rt, MC::SandboxCallback<ModuleDynamicImportHook> func) {
  return SetModuleDynamicImportHook(rt->rt_, func.UNSAFE_get());
}

inline bool FinishDynamicModuleImport(MCContext* cx,
                                      Handle<JSObject*> evaluationPromise,
                                      Handle<Value> referencingPrivate,
                                      Handle<JSObject*> moduleRequest,
                                      Handle<JSObject*> promise) {
  return FinishDynamicModuleImport(cx->cx_, evaluationPromise,
                                   referencingPrivate, moduleRequest, promise);
}

inline JSObject* CompileModule(MCContext* cx,
                               const ReadOnlyCompileOptions& options,
                               SourceText<char16_t>& srcBuf) {
  return CompileModule(cx->cx_, options, srcBuf);
}

inline JSObject* CompileModule(MCContext* cx,
                               const ReadOnlyCompileOptions& options,
                               SourceText<mozilla::Utf8Unit>& srcBuf) {
  return CompileModule(cx->cx_, options, srcBuf);
}

inline bool ModuleLink(MCContext* cx, Handle<JSObject*> moduleRecord) {
  return ModuleLink(cx->cx_, moduleRecord);
}

inline bool ModuleEvaluate(MCContext* cx, Handle<JSObject*> moduleRecord,
                           MutableHandleValue rval) {
  return ModuleEvaluate(cx->cx_, moduleRecord, rval);
}

inline bool ThrowOnModuleEvaluationFailure(
    MCContext* cx, Handle<JSObject*> evaluationPromise,
    ModuleErrorBehaviour errorBehaviour = ReportModuleErrorsAsync) {
  return ThrowOnModuleEvaluationFailure(cx->cx_, evaluationPromise, errorBehaviour);
}

inline uint32_t GetRequestedModulesCount(MCContext* cx,
                                         Handle<JSObject*> moduleRecord) {
  return GetRequestedModulesCount(cx->cx_, moduleRecord);
}

inline JSString* GetRequestedModuleSpecifier(MCContext* cx,
                                             Handle<JSObject*> moduleRecord,
                                             uint32_t index) {
  return GetRequestedModuleSpecifier(cx->cx_, moduleRecord, index);
}

inline void GetRequestedModuleSourcePos(MCContext* cx,
                                        Handle<JSObject*> moduleRecord,
                                        uint32_t index,
                                        MC::Tainted<uint32_t*> lineNumber,
                                        MC::Tainted<uint32_t*> columnNumber) {
  return GetRequestedModuleSourcePos(cx->cx_, moduleRecord, index,
                                     lineNumber.INTERNAL_unverified_safe(),
                                     columnNumber.INTERNAL_unverified_safe());
}

inline JSObject* CreateModuleRequest(MCContext* cx,
                                     Handle<JSString*> specifierArg) {
  return CreateModuleRequest(cx->cx_, specifierArg);
}

inline JSString* GetModuleRequestSpecifier(MCContext* cx,
                                           Handle<JSObject*> moduleRequestArg) {
  return GetModuleRequestSpecifier(cx->cx_, moduleRequestArg);
}

inline JSObject* GetModuleNamespace(MCContext* cx,
                                    Handle<JSObject*> moduleRecord) {
  return GetModuleNamespace(cx->cx_, moduleRecord);
}

inline JSObject* GetModuleForNamespace(MCContext* cx,
                                       Handle<JSObject*> moduleNamespace) {
  return GetModuleForNamespace(cx->cx_, moduleNamespace);
}

inline JSObject* GetModuleEnvironment(MCContext* cx,
                                      Handle<JSObject*> moduleObj) {
  return GetModuleEnvironment(cx->cx_, moduleObj);
}
}  // namespace JS
#endif

#endif
