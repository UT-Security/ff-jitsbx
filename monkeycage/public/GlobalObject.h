/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_GlobalObject_h
#define mc_GlobalObject_h

#include "js/GlobalObject.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

inline JSObject* JS_NewGlobalObject(MCContext* cx, const JSClass* clasp,
                                    JSPrincipals* principals,
                                    JS::OnNewGlobalHookOption hookOption,
                                    const JS::RealmOptions& options) {
  return JS_NewGlobalObject(cx->cx_, clasp, principals, hookOption, options);
}

inline void JS_FireOnNewGlobalObject(MCContext* cx, JS::HandleObject global) {
  return JS_FireOnNewGlobalObject(cx->cx_, global);
}
#endif

namespace MC {
extern const JSClassOps* DefaultGlobalClassOps();
}

#endif
