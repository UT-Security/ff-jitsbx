/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_GCAPI_h
#define monkeycage_GCAPI_h

#include "Tainted.h"
#include "js/GCAPI.h"
#include "monkeycage/Tainted.h"

namespace MC {
#ifdef DEBUG
using AutoAssertNoGC = monkeycage::AutoStackTainted<JS::AutoAssertNoGC>;
using AutoSuppressGCAnalysis = monkeycage::AutoStackTainted<JS::AutoSuppressGCAnalysis>;
using AutoAssertGCCallback = monkeycage::AutoStackTainted<JS::AutoAssertGCCallback>;
using AutoCheckCannotGC = monkeycage::AutoStackTainted<JS::AutoCheckCannotGC>;
#else
using AutoAssertNoGC = JS::AutoAssertNoGC;
using AutoSuppressGCAnalysis = JS::AutoSuppressGCAnalysis;
using AutoAssertGCCallback = JS::AutoAssertGCCallback;
using AutoCheckCannotGC = JS::AutoCheckCannotGC;
#endif
}  // namespace MC

#endif
