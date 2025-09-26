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

inline already_AddRefed<Stencil> CompileGlobalScriptToStencil(
    MCContext* cx, MC::Tainted<JS::OwningCompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf) {
  return CompileGlobalScriptToStencil(cx->cx_, *options.UNSAFE_unverified(),
                                      *srcBuf.UNSAFE_unverified());
}

inline already_AddRefed<Stencil> CompileGlobalScriptToStencil(
    MCContext* cx, MC::Tainted<JS::OwningCompileOptions*> options,
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
//   Instantiation
// ************************************************************************

namespace JS {

// Instantiate the Stencil into current Realm and return the JSScript.
inline JSScript* InstantiateGlobalStencil(
    MCContext* cx, MC::Tainted<InstantiateOptions*> options, Stencil* stencil,
    InstantiationStorage* storage = nullptr) {
  return InstantiateGlobalStencil(cx->cx_, *options.INTERNAL_unverified_safe(),
                                  stencil, storage);
}

// Instantiate a module Stencil and return the associated object. Inside the
// engine this is a js::ModuleObject.
inline JSObject* InstantiateModuleStencil(
    MCContext* cx, MC::Tainted<InstantiateOptions*> options, Stencil* stencil,
    InstantiationStorage* storage = nullptr) {
  return InstantiateModuleStencil(cx->cx_, *options.INTERNAL_unverified_safe(),
                                  stencil, storage);
}

}  // namespace JS

// ************************************************************************
//   Transcoding
// ************************************************************************

namespace JS {

// Serialize the Stencil into the transcode buffer.
inline TranscodeResult EncodeStencil(MCContext* cx, Stencil* stencil,
                                     MC::Tainted<TranscodeBuffer*> buffer) {
  return EncodeStencil(cx->cx_, stencil, *buffer.INTERNAL_unverified_safe());
}

// Deserialize data and create a new Stencil.
inline TranscodeResult DecodeStencil(MCContext* cx,
                                     MC::Tainted<DecodeOptions*> options,
                                     const TranscodeRange& range,
                                     Stencil** stencilOut) {
  return DecodeStencil(cx->cx_, *options.INTERNAL_unverified_safe(), range,
                       stencilOut);
}

// Register an encoder on its script source, such that all functions can be
// encoded as they are delazified.
inline bool StartIncrementalEncoding(MCContext* cx, RefPtr<Stencil>&& stencil) {
  return StartIncrementalEncoding(cx->cx_, std::forward<RefPtr<Stencil>>(stencil));
}

}  // namespace JS

// ************************************************************************
//   Off-thread compilation/transcoding
// ************************************************************************

namespace JS {

inline MC::Tainted<OffThreadToken*> CompileToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(CompileToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                   *srcBuf.INTERNAL_unverified_safe(),
                                   callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> CompileToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(CompileToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                   *srcBuf.INTERNAL_unverified_safe(),
                                   callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> CompileToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::OwningCompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(CompileToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                   *srcBuf.INTERNAL_unverified_safe(),
                                   callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> CompileToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::OwningCompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(CompileToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                   *srcBuf.INTERNAL_unverified_safe(),
                                   callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> CompileModuleToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<char16_t>*> srcBuf,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(CompileModuleToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                         *srcBuf.INTERNAL_unverified_safe(),
                                         callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> CompileModuleToStencilOffThread(
    MCContext* cx, MC::Tainted<JS::CompileOptions*> options,
    MC::Tainted<SourceText<mozilla::Utf8Unit>*> srcBuf,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(CompileModuleToStencilOffThread(cx->cx_, *options.UNSAFE_unverified(),
                                         *srcBuf.INTERNAL_unverified_safe(),
                                         callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> DecodeStencilOffThread(
    MCContext* cx, MC::Tainted<DecodeOptions*> options,
    const TranscodeBuffer& buffer, size_t cursor,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(DecodeStencilOffThread(cx->cx_, *options.INTERNAL_unverified_safe(),
                                buffer, cursor, callback.UNSAFE_get(),
                                callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> DecodeStencilOffThread(
    MCContext* cx, MC::Tainted<DecodeOptions*> options,
    const TranscodeRange& range,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(DecodeStencilOffThread(cx->cx_, *options.INTERNAL_unverified_safe(),
                                range, callback.UNSAFE_get(), callbackData));
  return ret;
}

inline MC::Tainted<OffThreadToken*> DecodeMultiStencilsOffThread(
    MCContext* cx, MC::Tainted<DecodeOptions*> options,
    TranscodeSources& sources,
    MC::SandboxCallback<OffThreadCompileCallback> callback,
    void* callbackData) {
  MC::Tainted<OffThreadToken*> ret{nullptr};
  ret.assign_raw_pointer(DecodeMultiStencilsOffThread(
      cx->cx_, *options.INTERNAL_unverified_safe(), sources,
      callback.UNSAFE_get(), callbackData));
  return ret;
}

inline already_AddRefed<Stencil> FinishOffThreadStencil(
    MCContext* cx, MC::Tainted<OffThreadToken*> token,
    InstantiationStorage* storage = nullptr) {
  return FinishOffThreadStencil(cx->cx_, token.INTERNAL_unverified_safe(),
                                storage);
}

inline bool FinishDecodeMultiStencilsOffThread(
    MCContext* cx, MC::Tainted<OffThreadToken*> token,
    mozilla::Vector<RefPtr<Stencil>>* stencils) {
  return FinishDecodeMultiStencilsOffThread(cx->cx_, token.INTERNAL_unverified_safe(), stencils);
}

inline void CancelOffThreadToken(MCContext* cx, MC::Tainted<OffThreadToken*> token) {
  return CancelOffThreadToken(cx->cx_, token.INTERNAL_unverified_safe());
}
}  // namespace JS
#endif

#endif
