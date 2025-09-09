/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Types and functions related to the compilation of JavaScript off the
 * direct JSAPI-using thread.
 */

#ifndef mc_OffThreadScriptCompilation_h
#define mc_OffThreadScriptCompilation_h

#include "js/OffThreadScriptCompilation.h"

#ifdef JS_SANDBOX

#include "monkeycage/CompileOptions.h"
#include "monkeycage/Context.h"

namespace JS {

inline bool CanCompileOffThread(MCContext* cx,
                                MC::Tainted<CompileOptions*> options,
                                size_t length) {
  return CanCompileOffThread(cx->cx_, *options.INTERNAL_unverified_safe(),
                             length);
}

inline bool CanDecodeOffThread(MCContext* cx,
                               MC::Tainted<DecodeOptions*> options,
                               size_t length) {
  return CanDecodeOffThread(cx->cx_, *options.INTERNAL_unverified_safe(),
                            length);
}

}  // namespace JS

#endif

#endif
