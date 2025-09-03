/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Error-reporting APIs.
 */

#ifndef mc_ErrorReport_h
#define mc_ErrorReport_h

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

template <typename... Args>
inline void JS_ReportErrorASCII(MCContext* cx, Args... args) {
  return JS_ReportErrorASCII(cx->cx_, "%s", args...);
}

template <typename... Args>
inline void JS_ReportErrorLatin1(MCContext* cx, Args... args) {
  return JS_ReportErrorLatin1(cx->cx_, "%s", args...);
}

template <typename... Args>
inline void JS_ReportErrorUTF8(MCContext* cx, Args... args) {
  return JS_ReportErrorUTF8(cx->cx_, "%s", args...);
}

inline MOZ_COLD void JS_ReportOutOfMemory(MCContext* cx) {
  return JS_ReportOutOfMemory(cx->cx_); 
}

#endif

#endif
