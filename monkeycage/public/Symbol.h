/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Symbols. */

#ifndef mc_Symbol_h
#define mc_Symbol_h

#include "js/Symbol.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {

inline Symbol* NewSymbol(MCContext* cx, Handle<JSString*> description) {
  return NewSymbol(cx->cx_, description);
}

inline Symbol* GetSymbolFor(MCContext* cx, Handle<JSString*> key) {
  return GetSymbolFor(cx->cx_, key);
}

inline Symbol* GetWellKnownSymbol(MCContext* cx, SymbolCode which) {
  return GetWellKnownSymbol(cx->cx_, which);
}

}  // namespace JS

#endif

#endif
