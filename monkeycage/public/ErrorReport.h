/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Error-reporting APIs.
 */

#ifndef mc_ErrorReport_h
#define mc_ErrorReport_h

#include "SandboxCallback.h"
#include "js/ErrorReport.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Exception.h"

namespace MC {
namespace detail {

template <typename MC_Sbx>
class TaintedVolatile<JS::ErrorReportBuilder, MC_Sbx> {
 private:
  JS::ErrorReportBuilder data;

 public:
  bool init(MCContext* cx, MC::Tainted<const JS::ExceptionStack*> exnStack,
            JS::ErrorReportBuilder::SniffingBehavior sniffingBehavior) {
    return data.init(cx->cx_, *exnStack.UNSAFE_unverified(), sniffingBehavior);
  }

  JSErrorReport* report() const { return data.report(); }

  const JS::ConstUTF8CharsZ toStringResult() const {
    return data.toStringResult();
  }
};

}  // namespace detail

}  // namespace MC

//TODO: allow callback to be either a sandbox address or a sandbox callback.
template <typename... Args>
inline void JS_ReportErrorNumberASCII(
    MCContext* cx, JSErrorCallback errorCallback,
    void* userRef, const unsigned errorNumber, Args... args) {
  return JS_ReportErrorNumberASCII(cx->cx_, errorCallback, userRef,
                                   errorNumber, args...);
}

inline MOZ_COLD void JS_ReportOutOfMemory(MCContext* cx) {
  return JS_ReportOutOfMemory(cx->cx_); 
}

namespace JS {

inline bool CreateError(
    MCContext* cx, JSExnType type, HandleObject stack, HandleString fileName,
    uint32_t lineNumber, uint32_t columnNumber, JSErrorReport* report,
    HandleString message, Handle<mozilla::Maybe<Value>> cause,
    MutableHandleValue rval) {
  return CreateError(cx->cx_, type, stack, fileName, lineNumber, columnNumber, report, message, cause, rval);
}

} /* namespace JS */
#endif

#endif
