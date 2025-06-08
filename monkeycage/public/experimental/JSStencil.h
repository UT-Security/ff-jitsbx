/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_experimental_JSStencil_h
#define mc_experimental_JSStencil_h

#include "js/experimental/JSStencil.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/CompileOptions.h"
#include "monkeycage/SourceText.h"

#include "monkeycage/Tainted.h"

// ************************************************************************
//   Compilation
// ************************************************************************

namespace JS {
inline already_AddRefed<Stencil> CompileGlobalScriptToStencil(
    MCContext* cx, MC::Tainted<const CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf) {
  return CompileGlobalScriptToStencil(cx->cx_, *options.UNSAFE_unverified(), *srcBuf.UNSAFE_unverified());
}

inline already_AddRefed<Stencil> CompileGlobalScriptToStencil(
    MCContext* cx, MC::Tainted<const CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf) {
  return CompileGlobalScriptToStencil(cx->cx_, *options.UNSAFE_unverified(), *srcBuf.UNSAFE_unverified());
}
}  // namespace JS

// ************************************************************************
//   Off-thread compilation/transcoding
// ************************************************************************

namespace JS {
inline void CancelOffThreadToken(MCContext* cx, OffThreadToken* token) {
  return CancelOffThreadToken(cx->cx_, token);
}
}  // namespace JS
#endif

#endif
