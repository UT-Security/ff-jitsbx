/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Functions to print out values during debugging. */

#ifndef mc_friend_DumpFunctions_h
#define mc_friend_DumpFunctions_h

#include "js/friend/DumpFunctions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {

/** Exposed for DumpJSStack */
inline JS::UniqueChars FormatStackDump(MCContext* cx,
                                                     bool showArgs,
                                                     bool showLocals,
                                                     bool showThisProps) {
  return FormatStackDump(cx->cx_, showArgs, showLocals, showThisProps);
}

}  // namespace JS

#endif

#endif
