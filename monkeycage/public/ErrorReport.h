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
    return data.init(cx->cx_, *exnStack.INTERNAL_unverified_safe(), sniffingBehavior);
  }

  MC::Tainted<JSErrorReport*> report() const {
    MC::Tainted<JSErrorReport*> ret{nullptr};
    ret.assign_raw_pointer(data.report());
    return ret;
  }

  const JS::ConstUTF8CharsZ toStringResult() const {
    return data.toStringResult();
  }
};

template <typename MC_Sbx>
class TaintedVolatile<JSErrorBase, MC_Sbx> {
 private:
  JSErrorBase data;

 public:
  // The UTF-8 encoded source file name, URL, etc., or null.
  const char* filename() { return data.filename; }

  // Unique identifier for the script source.
  unsigned sourceId() { return data.sourceId; }

  // Source line number.
  unsigned lineno() { return data.lineno; }

  // Zero-based column index in line.
  unsigned column() { return data.column; }

  // the error number, e.g. see js/public/friend/ErrorNumbers.msg.
  unsigned errorNumber() { return data.errorNumber; }

  // Points to JSErrorFormatString::name.
  // This string must never be freed.
  const char* errorMessageName() { return data.errorMessageName; }

  const JS::ConstUTF8CharsZ message() const { return data.message(); }

  void initOwnedMessage(const char* messageArg) {
    data.initOwnedMessage(messageArg);
  }
  void initBorrowedMessage(const char* messageArg) {
    data.initBorrowedMessage(messageArg);
  }

  JSString* newMessageString(MCContext* cx) {
    return data.newMessageString(cx->cx_);
  }
};

template <typename MC_Sbx>
class TaintedVolatile<JSErrorReport, MC_Sbx> {
 private:
  JSErrorReport data;

 public:
  const JS::ConstUTF8CharsZ message() const { return data.message(); }

  const char* filename() const { return data.filename; }

  unsigned lineno() const { return data.lineno; }

  unsigned column() const { return data.column; }

  unsigned errorNumber() const { return data.errorNumber; }
  
  const char* errorMessageName() const { return data.errorMessageName; }
  
  js::UniquePtr<JSErrorNotes>& notes() { return data.notes; }

  int16_t exnType() const { return data.exnType; }

  bool isMuted() const { return data.isMuted; }
  
  const char16_t* linebuf() const { return data.linebuf(); }
  
  size_t linebufLength() const { return data.linebufLength(); }

  size_t tokenOffset() const { return data.tokenOffset(); }

  void initOwnedLinebuf(const char16_t* linebufArg, size_t linebufLengthArg,
                        size_t tokenOffsetArg) {
    return data.initOwnedLinebuf(linebufArg, linebufLengthArg, tokenOffsetArg);
  }

  void initBorrowedLinebuf(const char16_t* linebufArg, size_t linebufLengthArg,
                           size_t tokenOffsetArg) {
    return initBorrowedLinebuf(linebufArg, linebufLengthArg, tokenOffsetArg);
  }

  bool isWarning() const { return data.isWarning(); }
};

}  // namespace detail
}  // namespace MC

template <size_t N, typename... Args>
inline void JS_ReportErrorASCII(MCContext* cx, const char (&format)[N], Args... args) {
     return JS_ReportErrorASCII(cx->cx_, format, args...);
}

template <size_t N, typename... Args>
inline void JS_ReportErrorLatin1(MCContext* cx, const char (&format)[N], Args... args) {
     return JS_ReportErrorLatin1(cx->cx_, format, args...);
}

template <size_t N, typename... Args>
inline void JS_ReportErrorUTF8(MCContext* cx, const char (&format)[N], Args... args) {
     return  JS_ReportErrorUTF8(cx->cx_, format, args...);
}

//TODO: allow callback to be either a sandbox address or a sandbox callback.
template <typename... Args>
inline void JS_ReportErrorNumberASCII(
    MCContext* cx, JSErrorCallback errorCallback,
    void* userRef, const unsigned errorNumber, Args... args) {
  return JS_ReportErrorNumberASCII(cx->cx_, errorCallback, userRef,
                                   errorNumber, args...);
}

template <typename... Args>
inline void JS_ReportErrorNumberLatin1(MCContext* cx,
                                       JSErrorCallback errorCallback,
                                       void* userRef,
                                       const unsigned errorNumber,
                                       Args... args) {
  JS_ReportErrorNumberLatin1(cx->cx_, errorCallback, userRef, errorNumber,
                             args...);
}

#  ifdef va_start
inline void JS_ReportErrorNumberLatin1VA(
    MCContext* cx, MC::SandboxCallback<JSErrorCallback> errorCallback, void* userRef,
    const unsigned errorNumber, va_list ap) {
 return JS_ReportErrorNumberLatin1VA(cx->cx_, errorCallback.UNSAFE_get(), userRef, errorNumber, ap);
}
#endif

#ifdef va_start
inline void JS_ReportErrorNumberUTF8VA(
    MCContext* cx, MC::SandboxCallback<JSErrorCallback> errorCallback, void* userRef,
    const unsigned errorNumber, va_list ap) {
 return JS_ReportErrorNumberUTF8VA(cx->cx_, errorCallback.UNSAFE_get(), userRef, errorNumber, ap);
}
#endif

inline void JS_ReportErrorNumberUTF8Array(
    MCContext* cx, MC::SandboxCallback<JSErrorCallback> errorCallback,
    void* userRef, const unsigned errorNumber, const char** args) {
  return JS_ReportErrorNumberUTF8Array(cx->cx_, errorCallback.UNSAFE_get(),
                                       userRef, errorNumber, args);
}

template <typename... Args>
inline void JS_ReportErrorNumberUC(
    MCContext* cx, MC::SandboxCallback<JSErrorCallback> errorCallback,
    void* userRef, const unsigned errorNumber, Args... args) {
  return JS_ReportErrorNumberUC(cx->cx_, errorCallback.UNSAFE_get(), userRef,
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

inline bool CreateError(MCContext* cx, JSExnType type, HandleObject stack,
                        HandleString fileName,
                        MC::Tainted<uint32_t*> lineNumber,
                        MC::Tainted<uint32_t*> columnNumber,
                        JSErrorReport* report, HandleString message,
                        Handle<mozilla::Maybe<Value>> cause,
                        MutableHandleValue rval) {
  return CreateError(
      cx->cx_, type, stack, fileName, *lineNumber.INTERNAL_unverified_safe(),
      *columnNumber.INTERNAL_unverified_safe(), report, message, cause, rval);
}

} /* namespace JS */
#endif

#endif
