/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Functionality provided for the JSM component loader in Gecko, that requires
 * its own unique manner of global environment and currently requires assistance
 * from SpiderMonkey to do so.
 *
 * Embedders who aren't Gecko can ignore this header.
 */

#ifndef mc_friend_JSMEnvironment_h
#define mc_friend_JSMEnvironment_h

#include "js/friend/JSMEnvironment.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {

inline JSObject* NewJSMEnvironment(MCContext* cx) {
  return NewJSMEnvironment(cx->cx_);
}

inline bool ExecuteInJSMEnvironment(MCContext* cx, Handle<JSScript*> script,
                                    Handle<JSObject*> jsmEnv) {
  return ExecuteInJSMEnvironment(cx->cx_, script, jsmEnv);
}

inline bool ExecuteInJSMEnvironment(
    MCContext* cx, Handle<JSScript*> script, Handle<JSObject*> jsmEnv,
    Handle<StackGCVector<JSObject*>> targetObj) {
  return ExecuteInJSMEnvironment(cx->cx_, script, jsmEnv, targetObj);
}

inline JSObject* GetJSMEnvironmentOfScriptedCaller(MCContext* cx) {
  return GetJSMEnvironmentOfScriptedCaller(cx->cx_);
}

}  // namespace JS

#endif

#endif
