/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef monkeycage_Class_h
#define monkeycage_Class_h

#include "monkeycage/Sandbox.h"
#include "monkeycage/Tainted.h"
#include "js/Class.h"

namespace monkeycage {
#ifdef JS_SANDBOX_API
extern JS_PUBLIC_API const JSClass* GetFunctionClassPtr();
extern JS_PUBLIC_API const JSClass* GetFunctionExtendedClassPtr();
#endif
}

#endif
