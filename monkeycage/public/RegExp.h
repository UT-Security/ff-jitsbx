/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Regular expression-related operations. */

#ifndef mc_RegExp_h
#define mc_RegExp_h

#include "js/RegExp.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline JSObject* NewRegExpObject(MCContext* cx, const char* bytes, size_t length,
                          RegExpFlags flags) {
  return NewRegExpObject(cx->cx_, bytes, length, flags);
}

inline JSObject* NewUCRegExpObject(MCContext* cx, const char16_t* chars, size_t length,
                            RegExpFlags flags) {
  return NewUCRegExpObject(cx->cx_, chars, length, flags);
}

inline bool SetRegExpInput(MCContext* cx, Handle<JSObject*> obj,
                    Handle<JSString*> input) {
  return SetRegExpInput(cx->cx_, obj, input);
}

inline bool ClearRegExpStatics(MCContext* cx, Handle<JSObject*> obj) {
  return ClearRegExpStatics(cx->cx_, obj);
}

inline bool ExecuteRegExp(MCContext* cx, Handle<JSObject*> obj,
                   Handle<JSObject*> reobj, const char16_t* chars,
                   size_t length, MC::Tainted<size_t*> indexp, bool test,
                   MutableHandle<Value> rval) {
  return ExecuteRegExp(cx->cx_, obj, reobj, chars, length,
                       indexp.INTERNAL_unverified_safe(), test, rval);
}

inline bool ExecuteRegExpNoStatics(MCContext* cx, Handle<JSObject*> reobj,
                            const char16_t* chars, size_t length,
                            MC::Tainted<size_t*> indexp, bool test,
                            MutableHandle<Value> rval) {
  return ExecuteRegExpNoStatics(cx->cx_, reobj, chars, length,
                                indexp.INTERNAL_unverified_safe(), test, rval);
}

inline bool ObjectIsRegExp(MCContext* cx, Handle<JSObject*> obj,
                    MC::Tainted<bool*> isRegExp) {
  return ObjectIsRegExp(cx->cx_, obj, isRegExp.INTERNAL_unverified_safe());
}

inline RegExpFlags GetRegExpFlags(MCContext* cx, Handle<JSObject*> obj) {
  return GetRegExpFlags(cx->cx_, obj);
}

inline JSString* GetRegExpSource(MCContext* cx, Handle<JSObject*> obj) {
  return GetRegExpSource(cx->cx_, obj);
}

inline bool CheckRegExpSyntax(MCContext* cx, const char16_t* chars, size_t length,
                       RegExpFlags flags, MutableHandle<Value> error) {
  return CheckRegExpSyntax(cx->cx_, chars, length, flags, error);
}

}  // namespace JS

#endif

#endif
