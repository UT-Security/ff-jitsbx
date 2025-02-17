/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jsshell_js_h
#define jsshell_js_h

#include "mozilla/Atomics.h"
#include "mozilla/Maybe.h"
#include "mozilla/TimeStamp.h"

#include <stdint.h>

#include "js/CompileOptions.h"
#include "js/GCVector.h"


namespace js {
namespace shell {

// Reference counted file.
struct RCFile {
  FILE* fp;
  uint32_t numRefs;

  RCFile() : fp(nullptr), numRefs(0) {}
  explicit RCFile(FILE* fp) : fp(fp), numRefs(0) {}

  void acquire() { numRefs++; }

  // Starts out with a ref count of zero.
  static RCFile* create(JSContext* cx, const char* filename, const char* mode);

  void close();
  bool isOpen() const { return fp; }
  bool release();
};

// Shell command-line arguments and count.
extern int sArgc;
extern char** sArgv;

// Shell state set once at startup.
extern const char* selfHostedXDRPath;
extern bool encodeSelfHostedCode;
extern bool enableCodeCoverage;
extern bool enableDisassemblyDumps;
extern bool offthreadCompilation;
extern JS::DelazificationOption defaultDelazificationMode;
extern bool enableAsmJS;
extern bool enableWasm;
extern bool enableSharedMemory;
extern bool enableWasmBaseline;
extern bool enableWasmOptimizing;

#define WASM_FEATURE(NAME, ...) extern bool enableWasm##NAME;
JS_FOR_WASM_FEATURES(WASM_FEATURE, WASM_FEATURE, WASM_FEATURE);
#undef WASM_FEATURE

extern bool enableWasmVerbose;
extern bool enableTestWasmAwaitTier2;
extern bool enableSourcePragmas;
extern bool enableAsyncStacks;
extern bool enableAsyncStackCaptureDebuggeeOnly;
extern bool enableWeakRefs;
extern bool enableToSource;
extern bool enablePropertyErrorMessageFix;
extern bool enableIteratorHelpers;
extern bool enableShadowRealms;
extern bool enableArrayGrouping;
extern bool enableArrayFromAsync;
extern bool enableWellFormedUnicodeStrings;
extern bool enablePrivateClassFields;
extern bool enablePrivateClassMethods;
extern bool enableChangeArrayByCopy;
#ifdef ENABLE_NEW_SET_METHODS
extern bool enableNewSetMethods;
#endif
extern bool enableClassStaticBlocks;
extern bool enableImportAssertions;
#ifdef JS_GC_ZEAL
extern uint32_t gZealBits;
extern uint32_t gZealFrequency;
#endif
extern bool printTiming;
extern RCFile* gErrFile;
extern RCFile* gOutFile;
extern bool reportWarnings;
extern bool compileOnly;
extern bool disableOOMFunctions;
extern bool defaultToSameCompartment;

#ifdef DEBUG
extern bool dumpEntrainedVariables;
extern bool OOM_printAllocationCount;
#endif

extern bool useFdlibmForSinCosTan;

extern JS::UniqueChars processWideModuleLoadPath;

// Per-context shell state.
struct ShellContext {
  explicit ShellContext(JSContext* cx);
  ~ShellContext();
};

extern ShellContext* GetShellContext(JSContext* cx);

} /* namespace shell */
} /* namespace js */
#endif
