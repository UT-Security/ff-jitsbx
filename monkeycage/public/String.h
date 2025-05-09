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

#endif

#endif
