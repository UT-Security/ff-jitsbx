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

namespace JS {

inline JSScript* Compile(MCContext* cx, const MC::CompileOptions& options,
                         MC::SourceText<char16_t>& srcBuf) {
  return Compile(cx->cx_, options.inner_, srcBuf.inner_);
}

inline JSScript* Compile(MCContext* cx, const MC::CompileOptions& options,
                         MC::SourceText<mozilla::Utf8Unit>& srcBuf) {
  return Compile(cx->cx_, options.inner_, srcBuf.inner_);
}
}  // namespace JS

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
#endif

#endif
