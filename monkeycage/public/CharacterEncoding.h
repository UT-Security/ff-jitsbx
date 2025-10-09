/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_CharacterEncoding_h
#define mc_CharacterEncoding_h

#include "js/CharacterEncoding.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Utility.h"

namespace JS {

inline size_t DeflateStringToUTF8BufferWithSbxCopy(JSLinearString* src,
                                               mozilla::Span<char> dst) {
  char* sbx_dst = (char*)js_malloc(dst.LengthBytes());
  if (!sbx_dst) return false;

  size_t written = DeflateStringToUTF8Buffer(src, mozilla::Span<char>{sbx_dst, dst.Length()});
  memcpy(dst.Elements(), sbx_dst, dst.LengthBytes());
  js_free(sbx_dst);

  return written;
}

inline JS::UniqueChars EncodeNarrowToUtf8(MCContext* cx, const char* chars) {
  char* ret = EncodeNarrowToUtf8Unsafe(cx->cx_, chars);
  return JS::UniqueChars(ret);
}

}

inline JS::UniqueChars JS_EncodeStringToLatin1(MCContext* cx, JSString* str) {
  char* ret = JS_EncodeStringToLatin1Unsafe(cx->cx_, str);
  return JS::UniqueChars(ret);
}

inline JS::UniqueChars JS_EncodeStringToUTF8(MCContext* cx,
                                             JS::Handle<JSString*> str) {
  char* ret = JS_EncodeStringToUTF8Unsafe(cx->cx_, str);
  return JS::UniqueChars(ret);
}

#endif

#endif
