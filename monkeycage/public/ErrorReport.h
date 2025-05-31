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

struct MOZ_STACK_CLASS JS_PUBLIC_API ErrorReportBuilder {
  JS::ErrorReportBuilder inner_;

  explicit ErrorReportBuilder(MCContext* cx) : inner_(cx->cx_) {}
  ~ErrorReportBuilder() = default;

  bool init(MCContext* cx, const MC::ExceptionStack& exnStack,
            JS::ErrorReportBuilder::SniffingBehavior sniffingBehavior) {
    return inner_.init(cx->cx_, exnStack.inner_, sniffingBehavior);
  }

  JSErrorReport* report() const { return inner_.report(); }

  const JS::ConstUTF8CharsZ toStringResult() const {
    return inner_.toStringResult();
  }
};

}  // namespace MC

template <typename... Args>
inline void JS_ReportErrorASCII(MCContext* cx, const char* format,
                                Args... args) {
  return JS_ReportErrorASCII(cx->cx_, format, args...);
}

template <typename... Args>
inline void JS_ReportErrorLatin1(MCContext* cx, const char* format,
                                 Args... args) {
  return JS_ReportErrorLatin1(cx->cx_, format, args...);
}

template <typename... Args>
inline void JS_ReportErrorUTF8(MCContext* cx, const char* format,
                               Args... args) {
  return JS_ReportErrorUTF8(cx->cx_, format, args...);
}

inline MOZ_COLD void JS_ReportOutOfMemory(MCContext* cx) {
  return JS_ReportOutOfMemory(cx->cx_); 
}
#else

namespace MC {

using ErrorReportBuilder = JS::ErrorReportBuilder;

}

#endif


#endif
