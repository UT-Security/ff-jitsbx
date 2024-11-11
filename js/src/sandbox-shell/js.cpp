/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JS shell using sandbox. */

#include "mozilla/ScopeExit.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "jsapi.h"
#include "js/Initialization.h"

#include "sandbox-shell/jsshell.h"

using namespace mozilla;
using namespace js;
using namespace js::shell;

bool SetGlobalOptionsPreJSInit() {
  JS::DisableJitBackend();
  return true;
}

bool SetGlobalOptionsPostJSInit() {
  JS::DisableExtraThreads();
  return true;
}
  
extern "C" void sbx_init(void);

int main(int argc, char** argv) {
  sbx_init();
  //sArgc = argc;
  //sArgv = argv;

  int result;

  setlocale(LC_ALL, "");
  
  if (!SetGlobalOptionsPreJSInit()) {
    return EXIT_FAILURE;
  }

  if (const char* message = JS_InitWithFailureDiagnostic()) {
    printf("JS_Init failed: %s\n", message);
    return 1;
  }

  auto shutdownEngine = MakeScopeExit([] { JS_ShutDown(); });

  if (!SetGlobalOptionsPostJSInit()) {
    return EXIT_FAILURE;
  }

  /* Use the same parameters as the browser in xpcjsruntime.cpp. */
  JSContext* const cx = JS_NewContext(JS::DefaultHeapMaxBytes);
  if (!cx) {
    return 1;
  }

  auto destroyCx = MakeScopeExit([cx] { JS_DestroyContext(cx); });

  
 
  return 0;
}
