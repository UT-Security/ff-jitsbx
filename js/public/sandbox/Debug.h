/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Interfaces by which the embedding can interact with the Debugger API.

#ifndef js_sandbox_Debug_h
#define js_sandbox_Debug_h

#include "js/Debug.h"
#ifdef JS_SANDBOX_API
#include "js/sandbox/lib.h"
#endif

namespace JS {
namespace dbg {
namespace sandbox {

#ifdef JS_SANDBOX_API
class MOZ_STACK_CLASS JS_PUBLIC_API AutoEntryMonitor {
  JS::dbg::AutoEntryMonitorWithHooks base_;

public:
 virtual void Entry(JSContext* cx, JSFunction* function,
                                       HandleValue asyncStack,
                                       const char* asyncCause) = 0;

 static void FunctionEntryCb(void* p, JSContext* cx, JSFunction* function,
                             HandleValue asyncStack, const char* asyncCause) {
   auto* m = static_cast<AutoEntryMonitor*>(p);
   m->Entry(cx, function, asyncStack, asyncCause);
 }

 virtual void Entry(JSContext* cx, JSScript* script,
                                       HandleValue asyncStack,
                                       const char* asyncCause) = 0;

 static void ScriptEntryCb(void* p, JSContext* cx, JSScript* script,
                           HandleValue asyncStack, const char* asyncCause) {
   auto* m = static_cast<AutoEntryMonitor*>(p);
   m->Entry(cx, script, asyncStack, asyncCause);
 }

 virtual void Exit(JSContext* cx) {
   base_.JS::dbg::AutoEntryMonitor::Exit(cx);
 }

 static void ExitCb(void* p, JSContext* cx) {
   auto* m = static_cast<AutoEntryMonitor*>(p);
   m->Exit(cx);
 }

 static const JS::dbg::AutoEntryMonitorHooks* hooks() {
   static const JS::dbg::AutoEntryMonitorHooks __hooks = {
     (JS::dbg::AutoEntryMonitorFunctionEntryHook)sbx_register_cb((void*)FunctionEntryCb, 0),
     (JS::dbg::AutoEntryMonitorScriptEntryHook)sbx_register_cb((void*)ScriptEntryCb, 0),
     (JS::dbg::AutoEntryMonitorExitHook)sbx_register_cb((void*)ExitCb, 0),
   };
   return &__hooks;
 }

 AutoEntryMonitor(JSContext* cx) : base_(hooks(), this, cx) {}
};
#else
using AutoEntryMonitor = JS::dbg::AutoEntryMonitor;
#endif

} 
}
}
#endif
