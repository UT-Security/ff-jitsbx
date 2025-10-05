/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Functions and structures related to locale-sensitive behavior, including
 * exposure of the default locale (used by operations like toLocaleString).
 */

#ifndef mc_LocaleSensitive_h
#define mc_LocaleSensitive_h

#include "js/LocaleSensitive.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

inline bool JS_SetDefaultLocale(MCRuntime* rt, const char* locale) {
  return JS_SetDefaultLocale(rt->rt_, locale);
}

inline JS::UniqueChars JS_GetDefaultLocale(MCContext* cx) {
 return JS_GetDefaultLocale(cx->cx_);
}

// TODO(abhishek): Ensure callback registration at type level
inline void JS_SetLocaleCallbacks(MCRuntime* rt,
                                  const JSLocaleCallbacks* callbacks) {
  return JS_SetLocaleCallbacks(rt->rt_, callbacks);
}

/**
 * Return the current locale callbacks, which may be nullptr.
 */
inline const JSLocaleCallbacks* JS_GetLocaleCallbacks(
    MCRuntime* rt) {
  return JS_GetLocaleCallbacks(rt->rt_);
}

#endif

#endif

