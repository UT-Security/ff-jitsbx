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

inline JSString* JS_NewStringCopyUTF8Z(
    MCContext* cx, const JS::ConstUTF8CharsZ s) {
  return JS_NewStringCopyUTF8Z(cx->cx_, s);
}

inline JSString* JS_NewStringCopyUTF8N(MCContext* cx,
                                                     const JS::UTF8Chars s) {
  return JS_NewStringCopyUTF8N(cx->cx_, s);
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

inline JSString* JS_NewLatin1String(
    MCContext* cx, js::UniquePtr<JS::Latin1Char[], JS::FreePolicy> chars,
    size_t length) {
  return JS_NewLatin1StringUnsafe(cx->cx_, chars.release(), length);
}

inline JSString* JS_NewUCString(MCContext* cx, JS::UniqueTwoByteChars chars,
                                size_t length) {
  return JS_NewUCStringUnsafe(cx->cx_, chars.release(), length);
}

inline JSString* JS_NewUCStringDontDeflate(MCContext* cx,
                                           JS::UniqueTwoByteChars chars,
                                           size_t length) {
  return JS_NewUCStringDontDeflateUnsafe(cx->cx_, chars.release(), length);
}

inline JSString* JS_NewUCStringCopyN(MCContext* cx, const char16_t* s,
                                     size_t n) {
  return JS_NewUCStringCopyN(cx->cx_, s, n);
}

inline JSString* JS_NewUCStringCopyZ(MCContext* cx,
                                                   const char16_t* s) {
  return JS_NewUCStringCopyZ(cx->cx_, s);
}

inline JSString* JS_AtomizeUCStringN(MCContext* cx, const char16_t* s,
                                     size_t length) {
  return JS_AtomizeUCStringN(cx->cx_, s, length);
}

inline JSString* JS_AtomizeUCString(MCContext* cx, const char16_t* s) {
  return JS_AtomizeUCString(cx->cx_, s);
}

inline bool JS_CompareStrings(MCContext* cx, JSString* str1,
                                            JSString* str2, MC::Tainted<int32_t*> result) {
  return JS_CompareStrings(cx->cx_, str1, str2, result.INTERNAL_unverified_safe());
}

inline bool JS_StringEqualsAscii(MCContext* cx, JSString* str,
                                 const char* asciiBytes,
                                 MC::Tainted<bool*> match) {
  return JS_StringEqualsAscii(cx->cx_, str, asciiBytes,
                              match.INTERNAL_unverified_safe());
}

inline bool JS_StringEqualsAscii(MCContext* cx, JSString* str,
                                 const char* asciiBytes, size_t length,
                                 MC::Tainted<bool*> match) {
  return JS_StringEqualsAscii(cx->cx_, str, asciiBytes, length,
                              match.INTERNAL_unverified_safe());
}

template <size_t N>
inline bool JS_StringEqualsLiteral(MCContext* cx, JSString* str,
                                   const char (&asciiBytes)[N],
                                   MC::Tainted<bool*> match) {
  return JS_StringEqualsLiteral(cx->cx_, str, asciiBytes, match.INTERNAL_unverified_safe());
}

inline const JS::Latin1Char* JS_GetLatin1StringCharsAndLength(
    MCContext* cx, MC::Tainted<const JS::AutoCheckCannotGC*> nogc,
    JSString* str, MC::Tainted<size_t*> length) {
  return JS_GetLatin1StringCharsAndLength(cx->cx_, *nogc.UNSAFE_unverified(),
                                          str,
                                          length.INTERNAL_unverified_safe());
}

inline const char16_t* JS_GetTwoByteStringCharsAndLength(
    MCContext* cx, MC::Tainted<const JS::AutoCheckCannotGC*> nogc,
    JSString* str, MC::Tainted<size_t*> length) {
  return JS_GetTwoByteStringCharsAndLength(cx->cx_, *nogc.UNSAFE_unverified(), str,
                                           length.INTERNAL_unverified_safe());
}

inline bool JS_GetStringCharAt(MCContext* cx, JSString* str, size_t index,
                               MC::Tainted<char16_t*> res) {
  return JS_GetStringCharAt(cx->cx_, str, index,
                            res.INTERNAL_unverified_safe());
}

static inline bool JS_CopyStringCharsWithSbxCopy(MCContext* cx,
                                          mozilla::Range<char16_t> dest,
                                          JSString* str) {
  char16_t* sbx_buffer = (char16_t*)js_malloc(dest.length() * sizeof(char16_t));
  if (!sbx_buffer) return false;

  bool ret = JS_CopyStringChars(
      cx->cx_,
      mozilla::Range<char16_t>{sbx_buffer, dest.length()},
      str);

  if (!ret) {
    js_free(sbx_buffer);
    return false;
  }

  memcpy(dest.begin().get(), sbx_buffer, dest.length() * sizeof(char16_t));
  js_free(sbx_buffer);
  return true;
}

inline JSLinearString* JS_EnsureLinearString(MCContext* cx, JSString* str) {
  return JS_EnsureLinearString(cx->cx_, str);
}

inline size_t JS_GetStringEncodingLength(MCContext* cx, JSString* str) {
  return JS_GetStringEncodingLength(cx->cx_, str);
}

static inline bool JS_EncodeStringToBufferWithSbxCopy(MCContext* cx, JSString* str,
                                               char* buffer, size_t length) {
  char* sbx_buffer = (char*)js_malloc(length);
  if (!sbx_buffer) return false;

  bool ret = JS_EncodeStringToBuffer(cx->cx_, str, sbx_buffer, length);

  if(!ret) {
    js_free(sbx_buffer);
    return false;
  }

  memcpy(buffer, sbx_buffer, length);
  js_free(sbx_buffer);
  return true;
}

inline bool JS_EncodeStringToUTF8BufferPartial(MCContext* cx, JSString* str,
                                               mozilla::Span<char> buffer,
                                               MC::Tainted<size_t*> read,
                                               MC::Tainted<size_t*> written) {
  return JS_EncodeStringToUTF8BufferPartial(cx->cx_, str, buffer,
                                            read.INTERNAL_unverified_safe(),
                                            written.INTERNAL_unverified_safe());
}

static inline bool JS_EncodeStringToUTF8BufferPartialWithSbxCopy(MCContext* cx, JSString* str,
                                               mozilla::Span<char> buffer,
                                               MC::Tainted<size_t*> read,
                                               MC::Tainted<size_t*> written) {
  char* sbx_buffer = (char*)js_malloc(buffer.LengthBytes());
  if (!sbx_buffer) return false;

  bool ret = JS_EncodeStringToUTF8BufferPartial(cx->cx_, str, mozilla::Span<char>{sbx_buffer, buffer.Length()},
                                            read.INTERNAL_unverified_safe(),
                                            written.INTERNAL_unverified_safe());
  if (!ret) {
    js_free(sbx_buffer);
  }

  memcpy(buffer.Elements(), sbx_buffer, buffer.LengthBytes());
  js_free(sbx_buffer);
  return ret;
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

inline bool CopyStringChars(MCContext* cx, char16_t* dest, JSString* s,
                            size_t len, size_t start = 0) {
  return CopyStringChars(cx->cx_, dest, s, len, start);
}
}
#endif

#endif
