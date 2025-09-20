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

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  void setBorrowBuffer(bool b) {
    data.borrowBuffer = b;
  }

  void setUsePinnedBytecode(bool b) {
    data.usePinnedBytecode = b;
  }

  

  void setAllocateInstantiationStorage(bool b) {
    data.allocateInstantiationStorage = b;
  }

  bool getNoScriptRval() {
    return data.noScriptRval;
  }

  void setTopLevelAwait(bool b) {
    data.topLevelAwait = b;
  }

  void setDeoptimizeModuleGlobalVars(bool b) {
    data.deoptimizeModuleGlobalVars = b;
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
  
  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setEagerDelazificationStrategy(
      JS::DelazificationOption strategy) {
    data.setEagerDelazificationStrategy(strategy);
    return *this;
  }
  
  JS::DelazificationOption eagerDelazificationStrategy() const {
    return data.eagerDelazificationStrategy();
  }
  
  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setForceStrictMode() {
    data.setForceStrictMode();
    return *this;
  }

  TaintedVolatile<JS::CompileOptions, MC_Sbx>& setModule() {
    data.setModule();
    return *this;
  }
};


template <typename MC_Sbx>
class TaintedVolatile<JS::InstantiateOptions, MC_Sbx> {
 private:
  JS::InstantiateOptions data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }


  bool getDeferDebugMetadata() {
    return data.deferDebugMetadata;
  }
#ifdef DEBUG
  void assertDefault() const {
    data.assertDefault();
  }
#endif
};

template <typename MC_Sbx>
class TaintedVolatile<JS::DecodeOptions, MC_Sbx> {
 private:
  JS::DecodeOptions data;

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  void setBorrowBuffer(bool b) {
    data.borrowBuffer = b;
  }

  bool getBorrowBuffer() {
    return data.borrowBuffer;
  }

  bool getUsePinnedBytecode() {
    return data.usePinnedBytecode;
  }
};

}  // namespace detail
}  // namespace MC
#endif

#endif
