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
#include "monkeycage/Principals.h"
#include "monkeycage/Tainted.h"

inline JSObject* JS_NewGlobalObject(MCContext* cx, const JSClass* clasp,
                                    MCPrincipals* principals,
                                    JS::OnNewGlobalHookOption hookOption,
                                    const MC::Tainted<JS::RealmOptions*> options) {
  return JS_NewGlobalObject(cx->cx_, clasp, principals ? principals->inner_ : nullptr, hookOption, *options.UNSAFE_unverified());
}

inline JSObject* JS_NewGlobalObject(MCContext* cx, const JSClass* clasp,
                                    JSPrincipals* principals,
                                    JS::OnNewGlobalHookOption hookOption,
                                    const MC::Tainted<JS::RealmOptions*> options, void* /* internal */) {
  return JS_NewGlobalObject(cx->cx_, clasp, principals, hookOption, *options.UNSAFE_unverified());
}

inline void JS_FireOnNewGlobalObject(MCContext* cx, JS::HandleObject global) {
  return JS_FireOnNewGlobalObject(cx->cx_, global);
}

namespace JS {
inline JSObject* CurrentGlobalOrNull(MCContext* cx) {
  return CurrentGlobalOrNull(cx->cx_);
}
}
#endif

namespace MC {
extern const JSClassOps* DefaultGlobalClassOps();
}

#endif
