/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_public_Object_h
#define mc_public_Object_h

#include "js/Object.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {


inline bool GetBuiltinClass(MCContext* cx, Handle<JSObject*> obj,
                                          MC::Tainted<js::ESClass*> cls) {
  return GetBuiltinClass(cx->cx_, obj, cls.INTERNAL_unverified_safe());
}

}

#endif

#endif
