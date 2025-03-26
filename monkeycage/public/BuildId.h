/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Embedding-provided build ID information, used by SpiderMonkey to tag cached
 * compilation data so that cached data can be reused when possible, or
 * discarded and regenerated if necessary.
 */

#ifndef monkeycage_BuildId_h
#define monkeycage_BuildId_h

#include "Tainted.h"
#include "monkeycage/Sandbox.h"
#include "monkeycage/Tainted.h"

#include "js/BuildId.h"

namespace MC_JS {
  
using BuildIdOp = bool (*)(monkeycage::TaintedUnchecked<JS::BuildIdCharVector*> buildId);

inline void SetProcessBuildIdOp(monkeycage::SandboxCallback<BuildIdOp> buildIdOp) {
  JS::SetProcessBuildIdOp(reinterpret_cast<JS::BuildIdOp>(buildIdOp.get()));
}

inline bool GetOptimizedEncodingBuildId(monkeycage::Tainted<JS::BuildIdCharVector*> buildId) {
  return JS::GetOptimizedEncodingBuildId(buildId.INTERNAL_unverified_safe());
}

inline bool GetScriptTranscodingBuildId(monkeycage::Tainted<JS::BuildIdCharVector*> buildId) {
  return JS::GetOptimizedEncodingBuildId(buildId.INTERNAL_unverified_safe());
}

}

#endif
