/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_unsafe_SandboxImpl_h
#define mc_unsafe_SandboxImpl_h

#if defined(JS_SANDBOX_NOOP)
#include "monkeycage/unsafe/SandboxNoop.h"
#elif defined(JS_SANDBOX_DYLIB)
#include "monkeycage/unsafe/SandboxDylib.h"
#else
#include "monkeycage/unsafe/SandboxNone.h"
#endif

#endif
