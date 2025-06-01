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
#endif

#endif
