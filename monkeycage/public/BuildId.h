/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Embedding-provided build ID information, used by SpiderMonkey to tag cached
 * compilation data so that cached data can be reused when possible, or
 * discarded and regenerated if necessary.
 */

#ifndef mc_BuildId_h
#define mc_BuildId_h

#include "js/BuildId.h"

#ifdef JS_SANDBOX

#include "monkeycage/SandboxCallback.h"
#include "monkeycage/Vector.h"

namespace JS {

inline void SetProcessBuildIdOp(MC::SandboxCallback<BuildIdOp> buildIdOp) {
  return SetProcessBuildIdOp(buildIdOp.UNSAFE_get());
}

inline bool GetOptimizedEncodingBuildId(MC::Tainted<BuildIdCharVector*> buildId) {
 return GetOptimizedEncodingBuildId(buildId.INTERNAL_unverified_safe());
}

inline bool GetScriptTranscodingBuildId(MC::Tainted<BuildIdCharVector*> buildId) {
 return GetScriptTranscodingBuildId(buildId.INTERNAL_unverified_safe());
}
}

#endif

#endif
