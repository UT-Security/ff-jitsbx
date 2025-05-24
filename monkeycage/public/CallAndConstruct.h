/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Call and construct API. */

#ifndef mc_CallAndConstruct_h
#define mc_CallAndConstruct_h

#include "js/CallAndConstruct.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/RootingAPI.h"

inline bool JS_CallFunctionName(MCContext* cx, JS::Handle<JSObject*> obj,
                                const char* name,
                                const JS::HandleValueArray& args,
                                MC::MutableHandle<JS::Value> rval) {
  return JS_CallFunctionName(cx->cx_, obj, name, args, rval.MC_INTERNAL_SAFE_get());
}
#endif

#endif
