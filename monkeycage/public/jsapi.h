/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript API. */

#ifndef mc_jsapi_h
#define mc_jsapi_h

#include "jsapi.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline bool DescribeScriptedCaller(JSContext* cx,
                                   monkeycage::Tainted<AutoFilename*> filename = monkeycage::Tainted<AutoFilename*>(nullptr),
                                   monkeycage::Tainted<unsigned*> lineno = monkeycage::Tainted<unsigned*>(nullptr),
                                   monkeycage::Tainted<unsigned*> column = monkeycage::Tainted<unsigned*>(nullptr)) {
  return DescribeScriptedCaller(cx, filename.UNSAFE_unverified(), lineno.UNSAFE_unverified(), column.UNSAFE_unverified());
}

}

#endif
