/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_CompileOptions_h
#define mc_CompileOptions_h

#include "js/CompileOptions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace MC {

namespace detail {

template <typename MC_Sbx>
class TaintedVolatile<JS::CompileOptions, MC_Sbx> {
 private:
  JS::CompileOptions data;

 public:
  void setBorrowBuffer(bool b) {
    data.borrowBuffer = b;
  }

  void setAllocateInstantiationStorage(bool b) {
    data.allocateInstantiationStorage = b;
  }
  
  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setFile(const char* f) {
    data.setFile(f);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setLine(unsigned l) {
    data.setLine(l);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setFileAndLine(const char* f,
                                                              unsigned l) {
    data.setFileAndLine(f, l);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setSourceMapURL(
      const char16_t* s) {
    data.setSourceMapURL(s);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setMutedErrors(bool mute) {
    data.setMutedErrors(mute);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setColumn(unsigned c) {
    data.setColumn(c);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setScriptSourceOffset(
      unsigned o) {
    data.setScriptSourceOffset(o);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setIsRunOnce(bool once) {
    data.setIsRunOnce(once);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setNoScriptRval(bool nsr) {
    data.setNoScriptRval(nsr);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setSkipFilenameValidation(bool b) {
    data.setSkipFilenameValidation(b);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setSelfHostingMode(bool shm) {
    data.setSelfHostingMode(shm);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setSourceIsLazy(bool l) {
    data.setSourceIsLazy(l);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setNonSyntacticScope(bool n) {
    data.setNonSyntacticScope(n);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setIntroductionType(const char* t) {
    data.setIntroductionType(t);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setDeferDebugMetadata(bool v = true) {
    data.setDeferDebugMetadata(v);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setHideScriptFromDebugger(bool v = true) {
    data.setHideScriptFromDebugger(v);
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setIntroductionInfoToCaller(
      MCContext* cx, const char* introductionType,
      JS::MutableHandle<JSScript*> introductionScript) {
    data.setIntroductionInfoToCaller(cx->cx_, introductionType,
                                     introductionScript);
    return *this;
  }
};

}  // namespace detail
}  // namespace MC
#endif

#endif
