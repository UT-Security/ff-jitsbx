/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Context_h
#define monkeycage_Context_h

#include "monkeycage/Tainted.h"
#include "js/Context.h"

inline monkeycage::Tainted<JSContext*> MC_JS_NewContext(
    uint32_t maxbytes, monkeycage::Tainted<JSRuntime*> parentRuntime) {
  monkeycage::Tainted<JSContext*> tcx(nullptr);
  tcx.assign_raw_pointer(JS_NewContext(maxbytes, parentRuntime.INTERNAL_unverified_safe()));

  return tcx;
}

#endif

