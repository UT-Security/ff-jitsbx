/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_CompileOptions_h
#define mc_CompileOptions_h

#include "js/CompileOptions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
namespace MC {
  
class MOZ_STACK_CLASS CompileOptions final {
public:
  JS::CompileOptions inner_;
  
  // Default options determined using the JSContext.
  explicit CompileOptions(MCContext* cx) : inner_(cx->cx_) {}

  CompileOptions& setFile(const char* f) {
    inner_.setFile(f);
    return *this;
  }

  CompileOptions& setLine(unsigned l) {
    inner_.setLine(l);
    return *this;
  }

  CompileOptions& setFileAndLine(const char* f, unsigned l) {
    inner_.setFileAndLine(f, l);
    return *this;
  }

  CompileOptions& setSourceMapURL(const char16_t* s) {
    inner_.setSourceMapURL(s);
    return *this;
  }

  CompileOptions& setMutedErrors(bool mute) {
    inner_.setMutedErrors(mute);
    return *this;
  }

  CompileOptions& setColumn(unsigned c) {
    inner_.setColumn(c);
    return *this;
  }

  CompileOptions& setScriptSourceOffset(unsigned o) {
    inner_.setScriptSourceOffset(o);
    return *this;
  }

  CompileOptions& setIsRunOnce(bool once) {
    inner_.setIsRunOnce(once);
    return *this;
  }

  CompileOptions& setNoScriptRval(bool nsr) {
    inner_.setNoScriptRval(nsr);
    return *this;
  }

  CompileOptions& setSkipFilenameValidation(bool b) {
    inner_.setSkipFilenameValidation(b);
    return *this;
  }

  CompileOptions& setSelfHostingMode(bool shm) {
    inner_.setSelfHostingMode(shm);
    return *this;
  }

  CompileOptions& setSourceIsLazy(bool l) {
    inner_.setSourceIsLazy(l);
    return *this;
  }

  CompileOptions& setNonSyntacticScope(bool n) {
    inner_.setNonSyntacticScope(n);
    return *this;
  }

  CompileOptions& setIntroductionType(const char* t) {
    inner_.setIntroductionType(t);
    return *this;
  }

  CompileOptions& setDeferDebugMetadata(bool v = true) {
    inner_.setDeferDebugMetadata(v);
    return *this;
  }

  CompileOptions& setHideScriptFromDebugger(bool v = true) {
    inner_.setHideScriptFromDebugger(v);
    return *this;
  }

  CompileOptions& setIntroductionInfo(const char* introducerFn,
                                      const char* intro, unsigned line,
                                      uint32_t offset) {
    inner_.setIntroductionInfo(introducerFn, intro, line, offset);
    return *this;
  }

  // Set introduction information according to any currently executing script.
  CompileOptions& setIntroductionInfoToCaller(
      MCContext* cx, const char* introductionType,
      JS::MutableHandle<JSScript*> introductionScript) {
    inner_.setIntroductionInfoToCaller(cx->cx_, introductionType, introductionScript);
    return *this;
  }

  CompileOptions& setDiscardSource() {
    inner_.setDiscardSource();
    return *this;
  }

  CompileOptions& setForceFullParse() {
    inner_.setForceFullParse();
    return *this;
  }

  CompileOptions& setEagerDelazificationStrategy(
      JS::DelazificationOption strategy) {
    inner_.setEagerDelazificationStrategy(strategy);
    return *this;
  }

  CompileOptions& setForceStrictMode() {
    inner_.setForceStrictMode();
    return *this;
  }

  CompileOptions& setModule() {
    inner_.setModule();
    return *this;
  }

  CompileOptions(const CompileOptions& rhs) = delete;
  CompileOptions& operator=(const CompileOptions& rhs) = delete;
};
}  // namespace MC
#else
namespace MC {
using CompileOptions = JS::CompileOptions;
}
#endif

#endif
