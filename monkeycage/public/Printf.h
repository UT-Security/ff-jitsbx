/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Printf_h
#define mc_Printf_h

#include "js/Printf.h"

#ifdef JS_SANDBOX

template <typename... Args>
inline JS::UniqueChars MC_smprintf(const char* fmt, Args... args) {
  char* ret = JS_smprintf_unsafe(fmt, args...);
  return JS::UniqueChars(ret);
}

template <typename... Args>
inline JS::UniqueChars MC_sprintf_append(JS::UniqueChars&& last, const char* fmt, Args... args) {
  char* ret = JS_sprintf_append_unsafe(last.release(), fmt, args...);
  return JS::UniqueChars(ret);
}

inline JS::UniqueChars MC_vsmprintf(const char* fmt, va_list ap) {
  char* ret = JS_vsmprintf_unsafe(fmt, ap);
  return JS::UniqueChars(ret);
}

#endif

#endif
