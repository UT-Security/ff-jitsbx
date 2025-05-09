/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Monkeycage API. */

#ifndef mc_ContextOptions_h
#define mc_ContextOptions_h

#include "js/ContextOptions.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {
inline ContextOptions& ContextOptionsRef(MCContext* cx) {
  //TODO(abhishekcs): Nullptr check here ?
  return ContextOptionsRef(cx->cx_);
}
}
#else
#endif

#endif
