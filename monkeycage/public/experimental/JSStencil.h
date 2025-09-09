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

#include "monkeycage/SandboxCallback.h"
#include "monkeycage/Tainted.h"

// ************************************************************************
//   Compilation
// ************************************************************************

namespace JS {
inline already_AddRefed<Stencil> CompileGlobalScriptToStencil(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf) {
  return CompileGlobalScriptToStencil(cx->cx_, *options.UNSAFE_unverified(),
                                      *srcBuf.UNSAFE_unverified());
}

inline already_AddRefed<Stencil> CompileGlobalScriptToStencil(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf) {
  return CompileGlobalScriptToStencil(cx->cx_, *options.UNSAFE_unverified(),
                                      *srcBuf.UNSAFE_unverified());
}

inline already_AddRefed<Stencil> CompileModuleScriptToStencil(
    MCContext* cx, MC::Tainted<CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf) {
  return CompileModuleScriptToStencil(cx->cx_, *options.UNSAFE_unverified(),
                                      *srcBuf.UNSAFE_unverified());
}

inline already_AddRefed<Stencil> CompileModuleScriptToStencil(
    MCContext* cx, MC::Tainted<CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf) {
  return CompileModuleScriptToStencil(cx->cx_, *options.UNSAFE_unverified(),
                                      *srcBuf.UNSAFE_unverified());
}

}  // namespace JS

// ************************************************************************
//   Off-thread compilation/transcoding
// ************************************************************************

namespace JS {

inline OffThreadToken* CompileToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf, MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  return CompileToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                   *srcBuf.INTERNAL_unverified_safe(),
                                   callback.UNSAFE_get(), callbackData);
}

inline OffThreadToken* CompileToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf, MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  return CompileToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                   *srcBuf.INTERNAL_unverified_safe(),
                                   callback.UNSAFE_get(), callbackData);
}

inline OffThreadToken* CompileModuleToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf, MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  return CompileModuleToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                         *srcBuf.INTERNAL_unverified_safe(),
                                         callback.UNSAFE_get(), callbackData);
}

inline OffThreadToken* CompileModuleToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf, MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  return CompileModuleToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                         *srcBuf.INTERNAL_unverified_safe(),
                                         callback.UNSAFE_get(), callbackData);
}

inline OffThreadToken* DecodeStencilOffThread(
    MCContext* cx, const DecodeOptions& options, const TranscodeBuffer& buffer,
    size_t cursor, MC::SandboxCallback<OffThreadCompileCallback> callback, void* callbackData) {
    return DecodeStencilOffThread(cx->cx_, options, buffer, cursor, callback.UNSAFE_get(), callbackData);
}

inline OffThreadToken* DecodeStencilOffThread(
    MCContext* cx, const DecodeOptions& options, const TranscodeRange& range,
    MC::SandboxCallback<OffThreadCompileCallback> callback, void* callbackData) {
    return DecodeStencilOffThread(cx->cx_, options, range, callback.UNSAFE_get(), callbackData);
}

inline OffThreadToken* DecodeMultiStencilsOffThread(
    MCContext* cx, const DecodeOptions& options, TranscodeSources& sources,
    MC::SandboxCallback<OffThreadCompileCallback> callback, void* callbackData) {
    return DecodeMultiStencilsOffThread(cx->cx_, options, sources, callback.UNSAFE_get(), callbackData);
}

inline void CancelOffThreadToken(MCContext* cx, OffThreadToken* token) {
  return CancelOffThreadToken(cx->cx_, token);
}
}  // namespace JS
#endif

#endif
