/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JS shell using sandbox. */

#include "mozilla/ArrayUtils.h"
#include "mozilla/Atomics.h"
#include "mozilla/Attributes.h"
#include "mozilla/DebugOnly.h"
#include "mozilla/EnumSet.h"
#include "mozilla/IntegerPrintfMacros.h"
#include "mozilla/mozalloc.h"
#include "mozilla/PodOperations.h"
#include "mozilla/RandomNum.h"
#include "mozilla/RefPtr.h"
#include "mozilla/ScopeExit.h"
#include "mozilla/Sprintf.h"
#include "mozilla/TimeStamp.h"
#include "mozilla/UniquePtrExtensions.h"  // UniqueFreePtr
#include "mozilla/Utf8.h"
#include "mozilla/Variant.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "jsapi.h"
#include "jsfriendapi.h"
#include "jstypes.h"
#include "js/BuildId.h"
#include "js/Initialization.h"

#include "sandbox-shell/jsshell.h"

using namespace js;
using namespace js::shell;

using JS::CompileOptions;

using js::shell::RCFile;

using mozilla::ArrayEqual;
using mozilla::AsVariant;
using mozilla::Atomic;
using mozilla::MakeScopeExit;
using mozilla::Maybe;
using mozilla::Nothing;
using mozilla::NumberEqualsInt32;
using mozilla::TimeDuration;
using mozilla::TimeStamp;
using mozilla::Utf8Unit;
using mozilla::Variant;

RCFile* shell::gErrFile = nullptr;
RCFile* shell::gOutFile = nullptr;

ShellContext::ShellContext(JSContext* cx) {}

ShellContext::~ShellContext() { }

ShellContext* js::shell::GetShellContext(JSContext* cx) {
  ShellContext* sc = static_cast<ShellContext*>(JS_GetContextPrivate(cx));
  MOZ_ASSERT(sc);
  return sc;
}

static bool ShellBuildId(JS::BuildIdCharVector* buildId);

static bool ShellBuildId(JS::BuildIdCharVector* buildId) {
  // The browser embeds the date into the buildid and the buildid is embedded
  // in the binary, so every 'make' necessarily builds a new firefox binary.
  // Fortunately, the actual firefox executable is tiny -- all the code is in
  // libxul.so and other shared modules -- so this isn't a big deal. Not so
  // for the statically-linked JS shell. To avoid recompiling js.cpp and
  // re-linking 'js' on every 'make', we use a constant buildid and rely on
  // the shell user to manually clear any caches between cache-breaking updates.
  const char buildid[] = "JS-shell";
  return buildId->append(buildid, sizeof(buildid));
}

// Used to allocate memory when jemalloc isn't yet initialized.
JS_DECLARE_NEW_METHODS(SystemAlloc_New, malloc, static)

static void SetOutputFile(const char* const envVar, RCFile* defaultOut,
                          RCFile** outFileP) {
  RCFile* outFile;

  const char* outPath = getenv(envVar);
  FILE* newfp;
  if (outPath && *outPath && (newfp = fopen(outPath, "w"))) {
    outFile = SystemAlloc_New<RCFile>(newfp);
  } else {
    outFile = defaultOut;
  }

  if (!outFile) {
    MOZ_CRASH("Failed to allocate output file");
  }

  outFile->acquire();
  *outFileP = outFile;
}

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

  RCFile rcStdout(stdout);
  rcStdout.acquire();
  RCFile rcStderr(stderr);
  rcStderr.acquire();

  SetOutputFile("JS_STDOUT", &rcStdout, &gOutFile);
  SetOutputFile("JS_STDERR", &rcStderr, &gErrFile);

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

  JS::SetProcessBuildIdOp(ShellBuildId);

  /* Use the same parameters as the browser in xpcjsruntime.cpp. */
  JSContext* const cx = JS_NewContext(JS::DefaultHeapMaxBytes);
  if (!cx) {
    return 1;
  }

  auto destroyCx = MakeScopeExit([cx] { JS_DestroyContext(cx); });

  UniquePtr<ShellContext> sc = MakeUnique<ShellContext>(cx);
  if (!sc) {
    return 1;
  }
  
  auto destroyShellContext = MakeScopeExit([cx, &sc] {
    // Must clear out some of sc's pointer containers before JS_DestroyContext.
    //sc->markObservers.reset();

    JS_SetContextPrivate(cx, nullptr);
    sc.reset();
  });

  JS_SetContextPrivate(cx, sc.get());
  
  return 0;
}
