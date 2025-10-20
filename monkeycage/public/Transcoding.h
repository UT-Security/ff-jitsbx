/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Structures and functions for transcoding compiled scripts and functions to
 * and from memory.
 */
#ifndef mc_Transcoding_h
#define mc_Transcoding_h

#include "js/Transcoding.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {

inline bool FinishIncrementalEncoding(MCContext* cx, Handle<JSScript*> script,
                                      MC::Tainted<TranscodeBuffer*> buffer) {
 return FinishIncrementalEncoding(cx->cx_, script, *buffer.INTERNAL_unverified_safe());
}

inline bool FinishIncrementalEncoding(MCContext* cx, Handle<JSObject*> module,
                                      MC::Tainted<TranscodeBuffer*> buffer) {
 return FinishIncrementalEncoding(cx->cx_, module, *buffer.INTERNAL_unverified_safe());
}
}

#endif
#endif
