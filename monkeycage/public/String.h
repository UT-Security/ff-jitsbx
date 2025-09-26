/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript string operations. */

#ifndef mc_String_h
#define mc_String_h

#include "js/String.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/GCAPI.h"
#include "monkeycage/Tainted.h"

inline JSString* JS_GetEmptyString(MCContext* cx) {
  return JS_GetEmptyString(cx->cx_);
}

inline JS::Value JS_GetEmptyStringValue(MCContext* cx) {
  return JS_GetEmptyStringValue(cx->cx_);
}

inline JSString* JS_NewStringCopyN(MCContext* cx, const char* s, size_t n) {
  return JS_NewStringCopyN(cx->cx_, s, n);
}

inline JSString* JS_NewStringCopyZ(MCContext* cx, const char* s) {
  return JS_NewStringCopyZ(cx->cx_, s);
}


inline JSString* JS_AtomizeStringN(MCContext* cx, const char* s,
                                                 size_t length) {
  return JS_AtomizeStringN(cx->cx_, s, length);
}

inline JSString* JS_AtomizeString(MCContext* cx, const char* s) {
  return JS_AtomizeString(cx->cx_, s);
}

inline JSString* JS_AtomizeAndPinStringN(MCContext* cx, const char* s,
                                         size_t length) {
  return JS_AtomizeAndPinStringN(cx->cx_, s, length);
}

// Note: unlike the non-pinning JS_Atomize* functions, this can be called
// without entering a realm/zone.
inline JSString* JS_AtomizeAndPinString(MCContext* cx, const char* s) {
  return JS_AtomizeAndPinString(cx->cx_, s);
}

inline JSString* JS_NewUCStringCopyN(MCContext* cx, const char16_t* s,
                                     size_t n) {
  return JS_NewUCStringCopyN(cx->cx_, s, n);
}

inline JSString* JS_NewUCStringCopyZ(MCContext* cx,
                                                   const char16_t* s) {
  return JS_NewUCStringCopyZ(cx->cx_, s);
}

template <size_t N>
inline bool JS_StringEqualsLiteral(MCContext* cx, JSString* str,
                                          const char (&asciiBytes)[N],
                                          MC::Tainted<bool*> match) {
  return JS_StringEqualsLiteral(cx->cx_, str, asciiBytes, match.INTERNAL_unverified_safe());
}

inline const JS::Latin1Char* JS_GetLatin1StringCharsAndLength(
    JSContext* cx, const MC::Tainted<JS::AutoCheckCannotGC*> nogc, JSString* str,
    size_t* length) {
  return JS_GetLatin1StringCharsAndLength(cx, *nogc.UNSAFE_unverified(), str, length);
}

inline const char16_t* JS_GetTwoByteStringCharsAndLength(
    JSContext* cx, const MC::Tainted<JS::AutoCheckCannotGC*> nogc, JSString* str,
    size_t* length) {
  return JS_GetTwoByteStringCharsAndLength(cx, *nogc.UNSAFE_unverified(), str, length);
}

inline JSLinearString* JS_EnsureLinearString(MCContext* cx, JSString* str) {
  return JS_EnsureLinearString(cx->cx_, str);
}

namespace JS {


MOZ_ALWAYS_INLINE const Latin1Char* GetLatin1LinearStringChars(
    const MC::Tainted<AutoCheckCannotGC*> nogc, JSLinearString* linear) {
  return GetLatin1LinearStringChars(*nogc.UNSAFE_unverified(), linear);
}

MOZ_ALWAYS_INLINE const char16_t* GetTwoByteLinearStringChars(
    const MC::Tainted<AutoCheckCannotGC*> nogc, JSLinearString* linear) {
  return GetTwoByteLinearStringChars(*nogc.UNSAFE_unverified(), linear);
}

MOZ_ALWAYS_INLINE bool IsExternalString(
    JSString* str, const MCExternalStringCallbacks** callbacks,
    const char16_t** chars) {
  const JSExternalStringCallbacks* callbacks_;
  bool ret = IsExternalString(str, &callbacks_, chars);
  if (!ret) {
    return ret;
  }

  *callbacks = static_cast<const MCExternalStringCallbacks*>(
      static_cast<const ::sandbox::JSExternalStringCallbacks*>(callbacks_)
          ->getExternalStringCallbacks());
  return ret;
}
}
#endif

#endif
