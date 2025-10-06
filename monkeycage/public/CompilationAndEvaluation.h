/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Functions for compiling and evaluating scripts. */

#ifndef mc_CompilationAndEvaluation_h
#define mc_CompilationAndEvaluation_h

#include "js/CompilationAndEvaluation.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/CompileOptions.h"
#include "monkeycage/SourceText.h"
#include "monkeycage/Tainted.h"

inline bool JS_Utf8BufferIsCompilableUnit(MCContext* cx,
                                          JS::Handle<JSObject*> obj,
                                          const char* utf8, size_t length) {
  return JS_Utf8BufferIsCompilableUnit(cx->cx_, obj, utf8, length);
}

inline bool JS_ExecuteScript(MCContext* cx, JS::Handle<JSScript*> script,
                             JS::MutableHandle<JS::Value> rval) {
  return JS_ExecuteScript(cx->cx_, script, rval);
}

inline bool JS_ExecuteScript(MCContext* cx, JS::Handle<JSScript*> script) {
  return JS_ExecuteScript(cx->cx_, script);
}

inline bool JS_ExecuteScript(MCContext* cx, JS::HandleObjectVector envChain,
                             JS::Handle<JSScript*> script,
                             JS::MutableHandle<JS::Value> rval) {
  return JS_ExecuteScript(cx->cx_, envChain, script, rval);
}

inline bool JS_ExecuteScript(MCContext* cx, JS::HandleObjectVector envChain,
                             JS::Handle<JSScript*> script) {
  return JS_ExecuteScript(cx->cx_, envChain, script);
}

namespace JS {

inline bool Evaluate(MCContext* cx,
                     MC::Tainted<const ReadOnlyCompileOptions*> options,
                     MC::Tainted<SourceText<char16_t>*> srcBuf,
                     MutableHandle<Value> rval) {
  return Evaluate(cx->cx_, *options.INTERNAL_unverified_safe(), *srcBuf.INTERNAL_unverified_safe(), rval);
}

inline bool Evaluate(MCContext* cx, MC::Tainted<const ReadOnlyCompileOptions*> options,
                     MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf,
                     MutableHandle<Value> rval) {
  return Evaluate(cx->cx_, *options.INTERNAL_unverified_safe(),
                  *srcBuf.INTERNAL_unverified_safe(), rval);
}

inline JSScript* Compile(MCContext* cx, MC::Tainted<const ReadOnlyCompileOptions*> options,
                         MC::Tainted<SourceText<char16_t>*> srcBuf) {
  return Compile(cx->cx_, *options.INTERNAL_unverified_safe(), *srcBuf.INTERNAL_unverified_safe());
}

inline JSScript* Compile(MCContext* cx, MC::Tainted<const ReadOnlyCompileOptions*> options,
                         MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf) {
  return Compile(cx->cx_, *options.INTERNAL_unverified_safe(), *srcBuf.INTERNAL_unverified_safe());
}

inline JSScript* CompileUtf8File(
    MCContext* cx, MC::Tainted<const ReadOnlyCompileOptions*> options, FILE* file) {
  return CompileUtf8File(cx->cx_, *options.INTERNAL_unverified_safe(), file);
}

inline JSScript* CompileUtf8Path(MCContext* cx,
                                 MC::Tainted<const ReadOnlyCompileOptions*> options,
                                 const char* filename) {
  return CompileUtf8Path(cx->cx_, *options.INTERNAL_unverified_safe(),
                         filename);
}

inline JSFunction* CompileFunction(
    MCContext* cx, HandleObjectVector envChain,
    MC::Tainted<const ReadOnlyCompileOptions*> options, const char* name,
    unsigned nargs, const char* const* argnames,
    MC::Tainted<SourceText<char16_t>*> srcBuf) {
  return CompileFunction(cx->cx_, envChain, *options.INTERNAL_unverified_safe(),
                         name, nargs, argnames,
                         *srcBuf.INTERNAL_unverified_safe());
}

inline JSFunction* CompileFunction(
    MCContext* cx, HandleObjectVector envChain,
    MC::Tainted<const ReadOnlyCompileOptions*> options, const char* name,
    unsigned nargs, const char* const* argnames,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf) {
  return CompileFunction(cx->cx_, envChain, *options.INTERNAL_unverified_safe(),
                         name, nargs, argnames,
                         *srcBuf.INTERNAL_unverified_safe());
}

inline void ExposeScriptToDebugger(MCContext* cx, Handle<JSScript*> script) {
  return ExposeScriptToDebugger(cx->cx_, script);
}

inline bool UpdateDebugMetadata(MCContext* cx, Handle<JSScript*> script,
                                MC::Tainted<const InstantiateOptions*> options,
                                HandleValue privateValue,
                                HandleString elementAttributeName,
                                HandleScript introScript,
                                HandleScript scriptOrModule) {
  return UpdateDebugMetadata(cx->cx_, script,
                             *options.INTERNAL_unverified_safe(), privateValue,
                             elementAttributeName, introScript, scriptOrModule);
}

}  // namespace JS

#endif

#endif
