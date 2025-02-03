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
#include "js/ContextOptions.h"
#include "js/Initialization.h"
#include "js/Utility.h"
#include "js/Vector.h"
#include "js/WasmFeatures.h"

//#include "gc/PublicIterators.h"
//#include "gc/Tracer.h"
//#include "vm/ArrayObject.h"

#include "sandbox-shell/jsoptparse.h"
#include "sandbox-shell/jsshell.h"

using namespace js;
using namespace js::cli;
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

#ifndef __wasi__
static const char sWasmCompileAndSerializeFlag[] =
    "--wasm-compile-and-serialize";
static Vector<const char*, 5, js::SystemAllocPolicy> sCompilerProcessFlags;
#endif

bool InitOptionParser(OptionParser& op);
bool SetGlobalOptionsPreJSInit(const OptionParser& op);
bool SetGlobalOptionsPostJSInit(const OptionParser& op);
bool SetContextOptions(JSContext* cx, const OptionParser& op);
bool SetContextWasmOptions(JSContext* cx, const OptionParser& op);
bool SetContextJITOptions(JSContext* cx, const OptionParser& op);
bool SetContextGCOptions(JSContext* cx, const OptionParser& op);
bool InitModuleLoader(JSContext* cx, const OptionParser& op);

// Not necessarily in sync with the browser
#ifdef ENABLE_SHARED_MEMORY
#  define SHARED_MEMORY_DEFAULT 1
#else
#  define SHARED_MEMORY_DEFAULT 0
#endif

bool shell::encodeSelfHostedCode = false;
bool shell::enableCodeCoverage = false;
bool shell::enableDisassemblyDumps = false;
bool shell::offthreadCompilation = false;
JS::DelazificationOption shell::defaultDelazificationMode =
    JS::DelazificationOption::OnDemandOnly;
bool shell::enableAsmJS = false;
bool shell::enableWasm = false;
bool shell::enableSharedMemory = SHARED_MEMORY_DEFAULT;
bool shell::enableWasmBaseline = false;
bool shell::enableWasmOptimizing = false;
#define WASM_DEFAULT_FEATURE(NAME, ...) bool shell::enableWasm##NAME = true;
#define WASM_EXPERIMENTAL_FEATURE(NAME, ...) \
  bool shell::enableWasm##NAME = false;
JS_FOR_WASM_FEATURES(WASM_DEFAULT_FEATURE, WASM_DEFAULT_FEATURE,
                     WASM_EXPERIMENTAL_FEATURE);
#undef WASM_DEFAULT_FEATURE
#undef WASM_EXPERIMENTAL_FEATURE

bool shell::enableWasmVerbose = false;
bool shell::enableTestWasmAwaitTier2 = false;
bool shell::enableSourcePragmas = true;
bool shell::enableAsyncStacks = false;
bool shell::enableAsyncStackCaptureDebuggeeOnly = false;
bool shell::enableWeakRefs = false;
bool shell::enableToSource = false;
bool shell::enablePropertyErrorMessageFix = false;
bool shell::enableIteratorHelpers = false;
bool shell::enableShadowRealms = false;
bool shell::enableArrayFromAsync = true;
#ifdef NIGHTLY_BUILD
bool shell::enableArrayGrouping = false;
// Pref for String.prototype.{is,to}WellFormed() methods.
bool shell::enableWellFormedUnicodeStrings = false;
#endif
bool shell::enableChangeArrayByCopy = false;
#ifdef ENABLE_NEW_SET_METHODS
bool shell::enableNewSetMethods = true;
#endif
bool shell::enableImportAssertions = false;
#ifdef JS_GC_ZEAL
uint32_t shell::gZealBits = 0;
uint32_t shell::gZealFrequency = 0;
#endif
bool shell::printTiming = false;
RCFile* shell::gErrFile = nullptr;
RCFile* shell::gOutFile = nullptr;
bool shell::reportWarnings = true;
bool shell::compileOnly = false;
bool shell::disableOOMFunctions = false;
bool shell::defaultToSameCompartment = true;

bool shell::useFdlibmForSinCosTan = false;

#ifdef DEBUG
bool shell::dumpEntrainedVariables = false;
bool shell::OOM_printAllocationCount = false;
#endif
JS::UniqueChars shell::processWideModuleLoadPath;

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

  OptionParser op("Usage: {progname} [options] [[script] scriptArgs*]");
  if (!InitOptionParser(op)) {
    return EXIT_FAILURE;
  }

  if (!SetGlobalOptionsPreJSInit(op)) {
    return EXIT_FAILURE;
  }

  if (const char* message = JS_InitWithFailureDiagnostic()) {
    printf("JS_Init failed: %s\n", message);
    return 1;
  }

  auto shutdownEngine = MakeScopeExit([] { JS_ShutDown(); });

  if (!SetGlobalOptionsPostJSInit(op)) {
    return EXIT_FAILURE;
  }

  JS::SetProcessBuildIdOp(ShellBuildId);

  /* Use the same parameters as the browser in xpcjsruntime.cpp. */
  //JSContext* const cx = JS_NewContext(JS::DefaultHeapMaxBytes);
  //if (!cx) {
  //  return 1;
  //}

  //auto destroyCx = MakeScopeExit([cx] { JS_DestroyContext(cx); });

  //UniquePtr<ShellContext> sc = MakeUnique<ShellContext>(cx);
  //if (!sc) {
  //  return 1;
  //}
  
  //auto destroyShellContext = MakeScopeExit([cx, &sc] {
    // Must clear out some of sc's pointer containers before JS_DestroyContext.
    //sc->markObservers.reset();

    //JS_SetContextPrivate(cx, nullptr);
    //sc.reset();
  //});

  //JS_SetContextPrivate(cx, sc.get());
  
  // Waiting is allowed on the shell's main thread, for now.
  //JS_SetFutexCanWait(cx);
  //JS::SetWarningReporter(cx, WarningReporter);

  return 0;
}

static bool OptionFailure(const char* option, const char* str) {
  fprintf(stderr, "Unrecognized option for %s: %s\n", option, str);
  return false;
}

bool InitOptionParser(OptionParser& op) {
  op.setDescription(
      "The SpiderMonkey shell provides a command line interface to the "
      "JavaScript engine. Code and file options provided via the command line "
      "are "
      "run left to right. If provided, the optional script argument is run "
      "after "
      "all options have been processed. Just-In-Time compilation modes may be "
      "enabled via "
      "command line options.");
  op.setDescriptionWidth(72);
  op.setHelpWidth(80);
  op.setVersion(JS_GetImplementationVersion());

  if (!op.addMultiStringOption(
          'f', "file", "PATH",
          "File path to run, parsing file contents as UTF-8") ||
      !op.addMultiStringOption(
          'u', "utf16-file", "PATH",
          "File path to run, inflating the file's UTF-8 contents to UTF-16 and "
          "then parsing that") ||
      !op.addMultiStringOption('m', "module", "PATH", "Module path to run") ||
      !op.addMultiStringOption('p', "prelude", "PATH", "Prelude path to run") ||
      !op.addMultiStringOption('e', "execute", "CODE", "Inline code to run") ||
      !op.addStringOption('\0', "selfhosted-xdr-path", "[filename]",
                          "Read/Write selfhosted script data from/to the given "
                          "XDR file") ||
      !op.addStringOption('\0', "selfhosted-xdr-mode", "(encode,decode,off)",
                          "Whether to encode/decode data of the file provided"
                          "with --selfhosted-xdr-path.") ||
      !op.addBoolOption('i', "shell", "Enter prompt after running code") ||
      !op.addBoolOption('c', "compileonly",
                        "Only compile, don't run (syntax checking mode)") ||
      !op.addBoolOption('w', "warnings", "Emit warnings") ||
      !op.addBoolOption('W', "nowarnings", "Don't emit warnings") ||
      !op.addBoolOption('D', "dump-bytecode",
                        "Dump bytecode with exec count for all scripts") ||
      !op.addBoolOption('b', "print-timing",
                        "Print sub-ms runtime for each file that's run") ||
      !op.addBoolOption('\0', "code-coverage",
                        "Enable code coverage instrumentation.") ||
      !op.addBoolOption(
          '\0', "disable-parser-deferred-alloc",
          "Disable deferred allocation of GC objects until after parser") ||
#ifdef DEBUG
      !op.addBoolOption('O', "print-alloc",
                        "Print the number of allocations at exit") ||
#endif
      !op.addOptionalStringArg("script",
                               "A script to execute (after all options)") ||
      !op.addOptionalMultiStringArg(
          "scriptArgs",
          "String arguments to bind as |scriptArgs| in the "
          "shell's global") ||
      !op.addIntOption(
          '\0', "cpu-count", "COUNT",
          "Set the number of CPUs (hardware threads) to COUNT, the "
          "default is the actual number of CPUs. The total number of "
          "background helper threads is the CPU count plus some constant.",
          -1) ||
      !op.addIntOption('\0', "thread-count", "COUNT", "Alias for --cpu-count.",
                       -1) ||
      !op.addBoolOption('\0', "ion", "Enable IonMonkey (default)") ||
      !op.addBoolOption('\0', "no-ion", "Disable IonMonkey") ||
      !op.addBoolOption('\0', "no-ion-for-main-context",
                        "Disable IonMonkey for the main context only") ||
      !op.addIntOption('\0', "inlining-entry-threshold", "COUNT",
                       "The minimum stub entry count before trial-inlining a"
                       " call",
                       -1) ||
      !op.addIntOption('\0', "small-function-length", "COUNT",
                       "The maximum bytecode length of a 'small function' for "
                       "the purpose of inlining.",
                       -1) ||
      !op.addBoolOption('\0', "only-inline-selfhosted",
                        "Only inline selfhosted functions") ||
      !op.addBoolOption('\0', "no-asmjs", "Disable asm.js compilation") ||
      !op.addStringOption(
          '\0', "wasm-compiler", "[option]",
          "Choose to enable a subset of the wasm compilers, valid options are "
          "'none', 'baseline', 'ion', 'optimizing', "
          "'baseline+ion', 'baseline+optimizing'.") ||
      !op.addBoolOption('\0', "wasm-verbose",
                        "Enable WebAssembly verbose logging") ||
      !op.addBoolOption('\0', "disable-wasm-huge-memory",
                        "Disable WebAssembly huge memory") ||
      !op.addBoolOption('\0', "test-wasm-await-tier2",
                        "Forcibly activate tiering and block "
                        "instantiation on completion of tier2") ||
#define WASM_DEFAULT_FEATURE(NAME, LOWER_NAME, COMPILE_PRED, COMPILER_PRED, \
                             FLAG_PRED, SHELL, ...)                         \
  !op.addBoolOption('\0', "no-wasm-" SHELL, "Disable wasm " SHELL "feature.") ||
#define WASM_TENTATIVE_FEATURE(NAME, LOWER_NAME, COMPILE_PRED, COMPILER_PRED, \
                               FLAG_PRED, SHELL, ...)                         \
  !op.addBoolOption('\0', "no-wasm-" SHELL,                                   \
                    "Disable wasm " SHELL "feature.") ||                      \
      !op.addBoolOption('\0', "wasm-" SHELL, "No-op.") ||
#define WASM_EXPERIMENTAL_FEATURE(NAME, LOWER_NAME, COMPILE_PRED,       \
                                  COMPILER_PRED, FLAG_PRED, SHELL, ...) \
  !op.addBoolOption('\0', "wasm-" SHELL,                                \
                    "Enable experimental wasm " SHELL "feature.") ||    \
      !op.addBoolOption('\0', "no-wasm-" SHELL, "No-op.") ||
      JS_FOR_WASM_FEATURES(WASM_DEFAULT_FEATURE, WASM_TENTATIVE_FEATURE,
                           WASM_EXPERIMENTAL_FEATURE)
#undef WASM_DEFAULT_FEATURE
#undef WASM_TENTATIVE_FEATURE
#undef WASM_EXPERIMENTAL_FEATURE
          !op.addBoolOption('\0', "no-native-regexp",
                            "Disable native regexp compilation") ||
      !op.addIntOption(
          '\0', "regexp-warmup-threshold", "COUNT",
          "Wait for COUNT invocations before compiling regexps to native code "
          "(default 10)",
          -1) ||
      !op.addBoolOption('\0', "trace-regexp-parser", "Trace regexp parsing") ||
      !op.addBoolOption('\0', "trace-regexp-assembler",
                        "Trace regexp assembler") ||
      !op.addBoolOption('\0', "trace-regexp-interpreter",
                        "Trace regexp interpreter") ||
      !op.addBoolOption('\0', "trace-regexp-peephole",
                        "Trace regexp peephole optimization") ||
      !op.addBoolOption('\0', "less-debug-code",
                        "Emit less machine code for "
                        "checking assertions under DEBUG.") ||
      !op.addBoolOption('\0', "disable-weak-refs", "Disable weak references") ||
      !op.addBoolOption('\0', "disable-tosource", "Disable toSource/uneval") ||
      !op.addBoolOption('\0', "disable-property-error-message-fix",
                        "Disable fix for the error message when accessing "
                        "property of null or undefined") ||
      !op.addBoolOption('\0', "enable-iterator-helpers",
                        "Enable iterator helpers") ||
      !op.addBoolOption('\0', "enable-shadow-realms", "Enable ShadowRealms") ||
      !op.addBoolOption('\0', "enable-array-grouping",
                        "Enable Array.grouping") ||
      !op.addBoolOption('\0', "enable-array-from-async",
                        "Enable Array.fromAsync") ||
      !op.addBoolOption('\0', "enable-well-formed-unicode-strings",
                        "Enable String.prototype.{is,to}WellFormed() methods"
                        "(Well-Formed Unicode Strings)") ||
      !op.addBoolOption('\0', "enable-change-array-by-copy",
                        "Enable change-array-by-copy methods") ||
      !op.addBoolOption('\0', "disable-change-array-by-copy",
                        "Disable change-array-by-copy methods") ||
#ifdef ENABLE_NEW_SET_METHODS
      !op.addBoolOption('\0', "enable-new-set-methods",
                        "Enable New Set methods") ||
      !op.addBoolOption('\0', "disable-new-set-methods",
                        "Disable New Set methods") ||
#else
      !op.addBoolOption('\0', "enable-new-set-methods", "no-op") ||
      !op.addBoolOption('\0', "disable-new-set-methods", "no-op") ||
#endif
      !op.addBoolOption('\0', "enable-top-level-await",
                        "Enable top-level await") ||
      !op.addBoolOption('\0', "enable-class-static-blocks",
                        "(no-op) Enable class static blocks") ||
      !op.addBoolOption('\0', "enable-import-assertions",
                        "Enable import assertions") ||
      !op.addStringOption('\0', "shared-memory", "on/off",
                          "SharedArrayBuffer and Atomics "
#if SHARED_MEMORY_DEFAULT
                          "(default: on, off to disable)"
#else
                          "(default: off, on to enable)"
#endif
                          ) ||
      !op.addStringOption('\0', "spectre-mitigations", "on/off",
                          "Whether Spectre mitigations are enabled (default: "
                          "off, on to enable)") ||
      !op.addStringOption('\0', "cache-ir-stubs", "on/off/call",
                          "Use CacheIR stubs (default: on, off to disable, "
                          "call to enable work-in-progress call ICs)") ||
      !op.addStringOption('\0', "ion-shared-stubs", "on/off",
                          "Use shared stubs (default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-scalar-replacement", "on/off",
                          "Scalar Replacement (default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-gvn", "[mode]",
                          "Specify Ion global value numbering:\n"
                          "  off: disable GVN\n"
                          "  on:  enable GVN (default)\n") ||
      !op.addStringOption(
          '\0', "ion-licm", "on/off",
          "Loop invariant code motion (default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-edgecase-analysis", "on/off",
                          "Find edge cases where Ion can avoid bailouts "
                          "(default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-pruning", "on/off",
                          "Branch pruning (default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-range-analysis", "on/off",
                          "Range analysis (default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-sink", "on/off",
                          "Sink code motion (default: off, on to enable)") ||
      !op.addStringOption('\0', "ion-optimization-levels", "on/off",
                          "No-op for fuzzing") ||
      !op.addStringOption('\0', "ion-loop-unrolling", "on/off",
                          "(NOP for fuzzers)") ||
      !op.addStringOption(
          '\0', "ion-instruction-reordering", "on/off",
          "Instruction reordering (default: off, on to enable)") ||
      !op.addStringOption(
          '\0', "ion-optimize-shapeguards", "on/off",
          "Eliminate redundant shape guards (default: on, off to disable)") ||
      !op.addStringOption(
          '\0', "ion-optimize-gcbarriers", "on/off",
          "Eliminate redundant GC barriers (default: on, off to disable)") ||
      !op.addStringOption('\0', "ion-iterator-indices", "on/off",
                          "Optimize property access in for-in loops "
                          "(default: on, off to disable)") ||
      !op.addBoolOption('\0', "ion-check-range-analysis",
                        "Range analysis checking") ||
      !op.addBoolOption('\0', "ion-extra-checks",
                        "Perform extra dynamic validation checks") ||
      !op.addStringOption(
          '\0', "ion-inlining", "on/off",
          "Inline methods where possible (default: on, off to disable)") ||
      !op.addStringOption(
          '\0', "ion-osr", "on/off",
          "On-Stack Replacement (default: on, off to disable)") ||
      !op.addBoolOption('\0', "disable-bailout-loop-check",
                        "Turn off bailout loop check") ||
      !op.addBoolOption('\0', "enable-watchtower",
                        "Enable Watchtower optimizations") ||
      !op.addBoolOption('\0', "disable-watchtower",
                        "Disable Watchtower optimizations") ||
      !op.addBoolOption('\0', "enable-ic-frame-pointers",
                        "Use frame pointers in all IC stubs") ||
      !op.addBoolOption('\0', "scalar-replace-arguments",
                        "Use scalar replacement to optimize ArgumentsObject") ||
      !op.addStringOption(
          '\0', "ion-limit-script-size", "on/off",
          "Don't compile very large scripts (default: on, off to disable)") ||
      !op.addIntOption('\0', "ion-warmup-threshold", "COUNT",
                       "Wait for COUNT calls or iterations before compiling "
                       "at the normal optimization level (default: 1000)",
                       -1) ||
      !op.addIntOption('\0', "ion-full-warmup-threshold", "COUNT",
                       "No-op for fuzzing", -1) ||
      !op.addStringOption(
          '\0', "ion-regalloc", "[mode]",
          "Specify Ion register allocation:\n"
          "  backtracking: Priority based backtracking register allocation "
          "(default)\n"
          "  testbed: Backtracking allocator with experimental features\n"
          "  stupid: Simple block local register allocation") ||
      !op.addBoolOption(
          '\0', "ion-eager",
          "Always ion-compile methods (implies --baseline-eager)") ||
      !op.addBoolOption('\0', "fast-warmup",
                        "Reduce warmup thresholds for each tier.") ||
      !op.addStringOption('\0', "ion-offthread-compile", "on/off",
                          "Compile scripts off thread (default: on)") ||
      !op.addStringOption('\0', "ion-parallel-compile", "on/off",
                          "--ion-parallel compile is deprecated. Use "
                          "--ion-offthread-compile.") ||
      !op.addBoolOption('\0', "baseline",
                        "Enable baseline compiler (default)") ||
      !op.addBoolOption('\0', "no-baseline", "Disable baseline compiler") ||
      !op.addBoolOption('\0', "baseline-eager",
                        "Always baseline-compile methods") ||
      !op.addIntOption(
          '\0', "baseline-warmup-threshold", "COUNT",
          "Wait for COUNT calls or iterations before baseline-compiling "
          "(default: 10)",
          -1) ||
      !op.addBoolOption('\0', "blinterp",
                        "Enable Baseline Interpreter (default)") ||
      !op.addBoolOption('\0', "no-blinterp", "Disable Baseline Interpreter") ||
      !op.addBoolOption('\0', "disable-jithints",
                        "Disable caching eager baseline compilation hints.") ||
      !op.addBoolOption(
          '\0', "emit-interpreter-entry",
          "Emit Interpreter entry trampolines (default under --enable-perf)") ||
      !op.addBoolOption(
          '\0', "no-emit-interpreter-entry",
          "Do not emit Interpreter entry trampolines (default).") ||
      !op.addBoolOption('\0', "blinterp-eager",
                        "Always Baseline-interpret scripts") ||
      !op.addIntOption(
          '\0', "blinterp-warmup-threshold", "COUNT",
          "Wait for COUNT calls or iterations before Baseline-interpreting "
          "(default: 10)",
          -1) ||
      !op.addIntOption(
          '\0', "trial-inlining-warmup-threshold", "COUNT",
          "Wait for COUNT calls or iterations before trial-inlining "
          "(default: 500)",
          -1) ||
      !op.addBoolOption(
          '\0', "non-writable-jitcode",
          "(NOP for fuzzers) Allocate JIT code as non-writable memory.") ||
      !op.addBoolOption(
          '\0', "no-sse3",
          "Pretend CPU does not support SSE3 instructions and above "
          "to test JIT codegen (no-op on platforms other than x86 and x64).") ||
      !op.addBoolOption(
          '\0', "no-ssse3",
          "Pretend CPU does not support SSSE3 [sic] instructions and above "
          "to test JIT codegen (no-op on platforms other than x86 and x64).") ||
      !op.addBoolOption(
          '\0', "no-sse41",
          "Pretend CPU does not support SSE4.1 instructions "
          "to test JIT codegen (no-op on platforms other than x86 and x64).") ||
      !op.addBoolOption('\0', "no-sse4", "Alias for --no-sse41") ||
      !op.addBoolOption(
          '\0', "no-sse42",
          "Pretend CPU does not support SSE4.2 instructions "
          "to test JIT codegen (no-op on platforms other than x86 and x64).") ||
#ifdef ENABLE_WASM_AVX
      !op.addBoolOption('\0', "enable-avx",
                        "No-op. AVX is enabled by default, if available.") ||
      !op.addBoolOption(
          '\0', "no-avx",
          "Pretend CPU does not support AVX or AVX2 instructions "
          "to test JIT codegen (no-op on platforms other than x86 and x64).") ||
#else
      !op.addBoolOption('\0', "enable-avx",
                        "AVX is disabled by default. Enable AVX. "
                        "(no-op on platforms other than x86 and x64).") ||
      !op.addBoolOption('\0', "no-avx",
                        "No-op. AVX is currently disabled by default.") ||
#endif
      !op.addBoolOption('\0', "more-compartments",
                        "Make newGlobal default to creating a new "
                        "compartment.") ||
      !op.addBoolOption('\0', "fuzzing-safe",
                        "Don't expose functions that aren't safe for "
                        "fuzzers to call") ||
#ifdef DEBUG
      !op.addBoolOption('\0', "differential-testing",
                        "Avoid random/undefined behavior that disturbs "
                        "differential testing (correctness fuzzing)") ||
#endif
      !op.addBoolOption('\0', "disable-oom-functions",
                        "Disable functions that cause "
                        "artificial OOMs") ||
      !op.addBoolOption('\0', "no-threads", "Disable helper threads") ||
      !op.addBoolOption(
          '\0', "no-jit-backend",
          "Disable the JIT backend completely for this process") ||
#ifdef DEBUG
      !op.addBoolOption('\0', "dump-entrained-variables",
                        "Print variables which are "
                        "unnecessarily entrained by inner functions") ||
#endif
      !op.addBoolOption('\0', "no-ggc", "Disable Generational GC") ||
      !op.addBoolOption('\0', "no-cgc", "Disable Compacting GC") ||
      !op.addBoolOption('\0', "no-incremental-gc", "Disable Incremental GC") ||
      !op.addBoolOption('\0', "enable-parallel-marking",
                        "Turn on parallel marking") ||
      !op.addIntOption(
          '\0', "marking-threads", "COUNT",
          "Set the number of threads used for parallel marking to COUNT.", 0) ||
      !op.addStringOption('\0', "nursery-strings", "on/off",
                          "Allocate strings in the nursery") ||
      !op.addStringOption('\0', "nursery-bigints", "on/off",
                          "Allocate BigInts in the nursery") ||
      !op.addIntOption('\0', "available-memory", "SIZE",
                       "Select GC settings based on available memory (MB)",
                       0) ||
      !op.addStringOption('\0', "arm-hwcap", "[features]",
                          "Specify ARM code generation features, or 'help' to "
                          "list all features.") ||
      !op.addIntOption('\0', "arm-asm-nop-fill", "SIZE",
                       "Insert the given number of NOP instructions at all "
                       "possible pool locations.",
                       0) ||
      !op.addIntOption('\0', "asm-pool-max-offset", "OFFSET",
                       "The maximum pc relative OFFSET permitted in pool "
                       "reference instructions.",
                       1024) ||
      !op.addBoolOption('\0', "arm-sim-icache-checks",
                        "Enable icache flush checks in the ARM "
                        "simulator.") ||
      !op.addIntOption('\0', "arm-sim-stop-at", "NUMBER",
                       "Stop the ARM simulator after the given "
                       "NUMBER of instructions.",
                       -1) ||
      !op.addBoolOption('\0', "mips-sim-icache-checks",
                        "Enable icache flush checks in the MIPS "
                        "simulator.") ||
      !op.addIntOption('\0', "mips-sim-stop-at", "NUMBER",
                       "Stop the MIPS simulator after the given "
                       "NUMBER of instructions.",
                       -1) ||
      !op.addBoolOption('\0', "loong64-sim-icache-checks",
                        "Enable icache flush checks in the LoongArch64 "
                        "simulator.") ||
      !op.addIntOption('\0', "loong64-sim-stop-at", "NUMBER",
                       "Stop the LoongArch64 simulator after the given "
                       "NUMBER of instructions.",
                       -1) ||
#ifdef JS_CODEGEN_RISCV64
      !op.addBoolOption('\0', "riscv-debug", "debug print riscv info.") ||
#endif
#ifdef JS_SIMULATOR_RISCV64
      !op.addBoolOption('\0', "trace-sim", "print simulator info.") ||
      !op.addBoolOption('\0', "debug-sim", "debug simulator.") ||
      !op.addBoolOption('\0', "riscv-trap-to-simulator-debugger",
                        "trap into simulator debuggger.") ||
      !op.addIntOption('\0', "riscv-sim-stop-at", "NUMBER",
                       "Stop the riscv simulator after the given "
                       "NUMBER of instructions.",
                       -1) ||
#endif
      !op.addIntOption('\0', "nursery-size", "SIZE-MB",
                       "Set the maximum nursery size in MB",
                       JS::DefaultNurseryMaxBytes / 1024 / 1024) ||
//#ifdef JS_GC_ZEAL
//      !op.addStringOption('z', "gc-zeal", "LEVEL(;LEVEL)*[,N]",
//                          gc::ZealModeHelpText) ||
//#else
      !op.addStringOption('z', "gc-zeal", "LEVEL(;LEVEL)*[,N]",
                          "option ignored in non-gc-zeal builds") ||
//#endif
      !op.addMultiStringOption('\0', "gc-param", "NAME=VALUE",
                               "Set a named GC parameter") ||
      !op.addStringOption('\0', "module-load-path", "DIR",
                          "Set directory to load modules from") ||
      !op.addBoolOption('\0', "no-source-pragmas",
                        "Disable source(Mapping)URL pragma parsing") ||
      !op.addBoolOption('\0', "no-async-stacks", "Disable async stacks") ||
      !op.addBoolOption('\0', "async-stacks-capture-debuggee-only",
                        "Limit async stack capture to only debuggees") ||
      !op.addMultiStringOption('\0', "dll", "LIBRARY",
                               "Dynamically load LIBRARY") ||
      !op.addBoolOption('\0', "suppress-minidump",
                        "Suppress crash minidumps") ||
#ifdef JS_ENABLE_SMOOSH
      !op.addBoolOption('\0', "smoosh", "Use SmooshMonkey") ||
      !op.addStringOption('\0', "not-implemented-watchfile", "[filename]",
                          "Track NotImplemented errors in the new frontend") ||
#else
      !op.addBoolOption('\0', "smoosh", "No-op") ||
#endif
      !op.addStringOption(
          '\0', "delazification-mode", "[option]",
          "Select one of the delazification mode for scripts given on the "
          "command line, valid options are: "
          "'on-demand', 'concurrent-df', 'eager', 'concurrent-df+on-demand'. "
          "Choosing 'concurrent-df+on-demand' will run both concurrent-df and "
          "on-demand delazification mode, and compare compilation outcome. ") ||
      !op.addBoolOption('\0', "wasm-compile-and-serialize",
                        "Compile the wasm bytecode from stdin and serialize "
                        "the results to stdout") ||
#ifdef FUZZING_JS_FUZZILLI
      !op.addBoolOption('\0', "reprl", "Enable REPRL mode for fuzzing") ||
#endif
      !op.addStringOption('\0', "telemetry-dir", "[directory]",
                          "Output telemetry results in a directory") ||
      !op.addBoolOption('\0', "use-fdlibm-for-sin-cos-tan",
                        "Use fdlibm for Math.sin, Math.cos, and Math.tan")) {
    return false;
  }

  op.setArgTerminatesOptions("script", true);
  op.setArgCapturesRest("scriptArgs");

  return true;
}

bool SetGlobalOptionsPreJSInit(const OptionParser& op) {
  // Note: DisableJitBackend must be called before JS_InitWithFailureDiagnostic.
  if (op.getBoolOption("no-jit-backend")) {
    JS::DisableJitBackend();
  }
  return true;
}

bool SetGlobalOptionsPostJSInit(const OptionParser& op) {
  if (op.getBoolOption("no-threads")) {
    JS::DisableExtraThreads();
  }

  // The fake CPU count must be set before initializing the Runtime,
  // which spins up the thread pool.
  int32_t cpuCount = op.getIntOption("cpu-count");  // What we're really setting
  if (cpuCount < 0) {
    cpuCount = op.getIntOption("thread-count");  // Legacy name
  }
  //if (cpuCount >= 0 && !SetFakeCPUCount(cpuCount)) {
  //  return false;
  //}
  return true;
}
bool SetContextOptions(JSContext* cx, const OptionParser& op) {
  if (!SetContextWasmOptions(cx, op) || !SetContextJITOptions(cx, op) ||
      !SetContextGCOptions(cx, op)) {
    return false;
  }

  enableSourcePragmas = !op.getBoolOption("no-source-pragmas");
  enableAsyncStacks = !op.getBoolOption("no-async-stacks");
  enableAsyncStackCaptureDebuggeeOnly =
      op.getBoolOption("async-stacks-capture-debuggee-only");
  enableWeakRefs = !op.getBoolOption("disable-weak-refs");
  enableToSource = !op.getBoolOption("disable-tosource");
  enablePropertyErrorMessageFix =
      !op.getBoolOption("disable-property-error-message-fix");
  enableIteratorHelpers = op.getBoolOption("enable-iterator-helpers");
  enableShadowRealms = op.getBoolOption("enable-shadow-realms");
  enableArrayFromAsync = op.getBoolOption("enable-array-from-async");
#ifdef NIGHTLY_BUILD
  enableArrayGrouping = op.getBoolOption("enable-array-grouping");
  enableWellFormedUnicodeStrings =
      op.getBoolOption("enable-well-formed-unicode-strings");
#endif
  enableChangeArrayByCopy = !op.getBoolOption("disable-change-array-by-copy");
#ifdef ENABLE_NEW_SET_METHODS
  enableNewSetMethods = op.getBoolOption("enable-new-set-methods");
#endif
  enableImportAssertions = op.getBoolOption("enable-import-assertions");
  useFdlibmForSinCosTan = op.getBoolOption("use-fdlibm-for-sin-cos-tan");

  JS::ContextOptionsRef(cx)
      .setSourcePragmas(enableSourcePragmas)
      .setAsyncStack(enableAsyncStacks)
      .setAsyncStackCaptureDebuggeeOnly(enableAsyncStackCaptureDebuggeeOnly)
      .setImportAssertions(enableImportAssertions);

  JS::SetUseFdlibmForSinCosTan(useFdlibmForSinCosTan);

  if (const char* str = op.getStringOption("shared-memory")) {
    if (strcmp(str, "off") == 0) {
      enableSharedMemory = false;
    } else if (strcmp(str, "on") == 0) {
      enableSharedMemory = true;
    } else {
      return OptionFailure("shared-memory", str);
    }
  }

  reportWarnings = op.getBoolOption('w');
  compileOnly = op.getBoolOption('c');
  printTiming = op.getBoolOption('b');
  enableDisassemblyDumps = op.getBoolOption('D');
  //cx->runtime()->profilingScripts =
  //    enableCodeCoverage || enableDisassemblyDumps;

  if (const char* mode = op.getStringOption("delazification-mode")) {
    if (strcmp(mode, "on-demand") == 0) {
      defaultDelazificationMode = JS::DelazificationOption::OnDemandOnly;
    } else if (strcmp(mode, "concurrent-df") == 0) {
      defaultDelazificationMode =
          JS::DelazificationOption::ConcurrentDepthFirst;
    } else if (strcmp(mode, "eager") == 0) {
      defaultDelazificationMode =
          JS::DelazificationOption::ParseEverythingEagerly;
    } else if (strcmp(mode, "concurrent-df+on-demand") == 0 ||
               strcmp(mode, "on-demand+concurrent-df") == 0) {
      defaultDelazificationMode =
          JS::DelazificationOption::CheckConcurrentWithOnDemand;
    } else {
      return OptionFailure("delazification-mode", mode);
    }
  }

  return true;
}

bool SetContextWasmOptions(JSContext* cx, const OptionParser& op) {
  enableAsmJS = !op.getBoolOption("no-asmjs");

  enableWasm = true;
  enableWasmBaseline = true;
  enableWasmOptimizing = true;

  if (const char* str = op.getStringOption("wasm-compiler")) {
    if (strcmp(str, "none") == 0) {
      enableWasm = false;
    } else if (strcmp(str, "baseline") == 0) {
      MOZ_ASSERT(enableWasmBaseline);
      enableWasmOptimizing = false;
    } else if (strcmp(str, "optimizing") == 0 ||
               strcmp(str, "optimized") == 0) {
      enableWasmBaseline = false;
      MOZ_ASSERT(enableWasmOptimizing);
    } else if (strcmp(str, "baseline+optimizing") == 0 ||
               strcmp(str, "baseline+optimized") == 0) {
      MOZ_ASSERT(enableWasmBaseline);
      MOZ_ASSERT(enableWasmOptimizing);
    } else if (strcmp(str, "ion") == 0) {
      enableWasmBaseline = false;
      enableWasmOptimizing = true;
    } else if (strcmp(str, "baseline+ion") == 0) {
      MOZ_ASSERT(enableWasmBaseline);
      enableWasmOptimizing = true;
    } else {
      return OptionFailure("wasm-compiler", str);
    }
  }

#define WASM_DEFAULT_FEATURE(NAME, LOWER_NAME, COMPILE_PRED, COMPILER_PRED, \
                             FLAG_PRED, SHELL, ...)                         \
  enableWasm##NAME = !op.getBoolOption("no-wasm-" SHELL);
#define WASM_EXPERIMENTAL_FEATURE(NAME, LOWER_NAME, COMPILE_PRED,       \
                                  COMPILER_PRED, FLAG_PRED, SHELL, ...) \
  enableWasm##NAME = op.getBoolOption("wasm-" SHELL);
  JS_FOR_WASM_FEATURES(WASM_DEFAULT_FEATURE, WASM_DEFAULT_FEATURE,
                       WASM_EXPERIMENTAL_FEATURE);
#undef WASM_DEFAULT_FEATURE
#undef WASM_EXPERIMENTAL_FEATURE

  enableWasmVerbose = op.getBoolOption("wasm-verbose");
  enableTestWasmAwaitTier2 = op.getBoolOption("test-wasm-await-tier2");

  JS::ContextOptionsRef(cx)
      .setAsmJS(enableAsmJS)
      .setWasm(enableWasm)
      .setWasmForTrustedPrinciples(enableWasm)
      .setWasmBaseline(enableWasmBaseline)
      .setWasmIon(enableWasmOptimizing)
#define WASM_FEATURE(NAME, ...) .setWasm##NAME(enableWasm##NAME)
          JS_FOR_WASM_FEATURES(WASM_FEATURE, WASM_FEATURE, WASM_FEATURE)
#undef WASM_FEATURE
      ;

#ifndef __wasi__
  // This must be set before self-hosted code is initialized, as self-hosted
  // code reads the property and the property may not be changed later.
  bool disabledHugeMemory = false;
  if (op.getBoolOption("disable-wasm-huge-memory")) {
    disabledHugeMemory = JS::DisableWasmHugeMemory();
    MOZ_RELEASE_ASSERT(disabledHugeMemory);
  }

  // --disable-wasm-huge-memory needs to be propagated.  See bug 1518210.
  if (disabledHugeMemory &&
      !sCompilerProcessFlags.append("--disable-wasm-huge-memory")) {
    return false;
  }

  // Also the following are to be propagated.
  const char* to_propagate[] = {
#  define WASM_DEFAULT_FEATURE(NAME, LOWER_NAME, COMPILE_PRED, COMPILER_PRED, \
                               FLAG_PRED, SHELL, ...)                         \
    "--no-wasm-" SHELL,
#  define WASM_EXPERIMENTAL_FEATURE(NAME, LOWER_NAME, COMPILE_PRED,       \
                                    COMPILER_PRED, FLAG_PRED, SHELL, ...) \
    "--wasm-" SHELL,
      JS_FOR_WASM_FEATURES(WASM_DEFAULT_FEATURE, WASM_DEFAULT_FEATURE,
                           WASM_EXPERIMENTAL_FEATURE)
#  undef WASM_DEFAULT_FEATURE
#  undef WASM_EXPERIMENTAL_FEATURE
      // Compiler selection options
      "--test-wasm-await-tier2",
      NULL};
  for (const char** p = &to_propagate[0]; *p; p++) {
    if (op.getBoolOption(&(*p)[2] /* 2 => skip the leading '--' */)) {
      if (!sCompilerProcessFlags.append(*p)) {
        return false;
      }
    }
  }

  // Also --wasm-compiler= is to be propagated.  This is tricky because it is
  // necessary to reconstitute the --wasm-compiler=<whatever> string from its
  // pieces, without causing a leak.  Hence it is copied into a static buffer.
  // This is thread-unsafe, but we're in `main()` and on the process' root
  // thread.  Also, we do this only once -- it wouldn't work properly if we
  // handled multiple --wasm-compiler= flags in a loop.
  const char* wasm_compiler = op.getStringOption("wasm-compiler");
  if (wasm_compiler) {
    size_t n_needed =
        2 + strlen("wasm-compiler") + 1 + strlen(wasm_compiler) + 1;
    const size_t n_avail = 128;
    static char buf[n_avail];
    // `n_needed` depends on the compiler name specified.  However, it can't
    // be arbitrarily long, since previous flag-checking should have limited
    // it to a set of known possibilities: "baseline", "ion",
    // "baseline+ion",  Still, assert this for safety.
    MOZ_RELEASE_ASSERT(n_needed < n_avail);
    memset(buf, 0, sizeof(buf));
    SprintfBuf(buf, n_avail, "--%s=%s", "wasm-compiler", wasm_compiler);
    if (!sCompilerProcessFlags.append(buf)) {
      return false;
    }
  }
#endif  // __wasi__

  return true;
}

bool SetContextJITOptions(JSContext* cx, const OptionParser& op) {
  // Check --fast-warmup first because it sets default warm-up thresholds. These
  // thresholds can then be overridden below by --ion-eager and other flags.
  if (op.getBoolOption("fast-warmup")) {
    //jit::JitOptions.setFastWarmUp();
  }

  if (op.getBoolOption("no-ion-for-main-context")) {
    JS::ContextOptionsRef(cx).setDisableIon();
  }

  if (const char* str = op.getStringOption("cache-ir-stubs")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableCacheIR = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableCacheIR = true;
    } else {
      return OptionFailure("cache-ir-stubs", str);
    }
  }

  if (const char* str = op.getStringOption("spectre-mitigations")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.spectreIndexMasking = true;
      //jit::JitOptions.spectreObjectMitigations = true;
      //jit::JitOptions.spectreStringMitigations = true;
      //jit::JitOptions.spectreValueMasking = true;
      //jit::JitOptions.spectreJitToCxxCalls = true;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.spectreIndexMasking = false;
      //jit::JitOptions.spectreObjectMitigations = false;
      //jit::JitOptions.spectreStringMitigations = false;
      //jit::JitOptions.spectreValueMasking = false;
      //jit::JitOptions.spectreJitToCxxCalls = false;
    } else {
      return OptionFailure("spectre-mitigations", str);
    }
  }

  if (const char* str = op.getStringOption("ion-scalar-replacement")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableScalarReplacement = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableScalarReplacement = true;
    } else {
      return OptionFailure("ion-scalar-replacement", str);
    }
  }

  if (op.getStringOption("ion-shared-stubs")) {
    // Dead option, preserved for now for potential fuzzer interaction.
  }

  if (const char* str = op.getStringOption("ion-gvn")) {
    if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableGvn = true;
    } else if (strcmp(str, "on") != 0 && strcmp(str, "optimistic") != 0 &&
               strcmp(str, "pessimistic") != 0) {
      // We accept "pessimistic" and "optimistic" as synonyms for "on"
      // for backwards compatibility.
      return OptionFailure("ion-gvn", str);
    }
  }

  if (const char* str = op.getStringOption("ion-licm")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableLicm = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableLicm = true;
    } else {
      return OptionFailure("ion-licm", str);
    }
  }

  if (const char* str = op.getStringOption("ion-edgecase-analysis")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableEdgeCaseAnalysis = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableEdgeCaseAnalysis = true;
    } else {
      return OptionFailure("ion-edgecase-analysis", str);
    }
  }

  if (const char* str = op.getStringOption("ion-pruning")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disablePruning = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disablePruning = true;
    } else {
      return OptionFailure("ion-pruning", str);
    }
  }

  if (const char* str = op.getStringOption("ion-range-analysis")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableRangeAnalysis = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableRangeAnalysis = true;
    } else {
      return OptionFailure("ion-range-analysis", str);
    }
  }

  if (const char* str = op.getStringOption("ion-sink")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableSink = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableSink = true;
    } else {
      return OptionFailure("ion-sink", str);
    }
  }

  if (const char* str = op.getStringOption("ion-optimize-shapeguards")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableRedundantShapeGuards = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableRedundantShapeGuards = true;
    } else {
      return OptionFailure("ion-optimize-shapeguards", str);
    }
  }

  if (const char* str = op.getStringOption("ion-optimize-gcbarriers")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableRedundantGCBarriers = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableRedundantGCBarriers = true;
    } else {
      return OptionFailure("ion-optimize-gcbarriers", str);
    }
  }

  if (const char* str = op.getStringOption("ion-instruction-reordering")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableInstructionReordering = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableInstructionReordering = true;
    } else {
      return OptionFailure("ion-instruction-reordering", str);
    }
  }

  if (op.getBoolOption("ion-check-range-analysis")) {
    //jit::JitOptions.checkRangeAnalysis = true;
  }

  if (op.getBoolOption("ion-extra-checks")) {
    //jit::JitOptions.runExtraChecks = true;
  }

  if (const char* str = op.getStringOption("ion-inlining")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableInlining = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableInlining = true;
    } else {
      return OptionFailure("ion-inlining", str);
    }
  }

  if (const char* str = op.getStringOption("ion-osr")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.osr = true;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.osr = false;
    } else {
      return OptionFailure("ion-osr", str);
    }
  }

  if (const char* str = op.getStringOption("ion-limit-script-size")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.limitScriptSize = true;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.limitScriptSize = false;
    } else {
      return OptionFailure("ion-limit-script-size", str);
    }
  }

  int32_t warmUpThreshold = op.getIntOption("ion-warmup-threshold");
  if (warmUpThreshold >= 0) {
    //jit::JitOptions.setNormalIonWarmUpThreshold(warmUpThreshold);
  }

  warmUpThreshold = op.getIntOption("baseline-warmup-threshold");
  if (warmUpThreshold >= 0) {
    //jit::JitOptions.baselineJitWarmUpThreshold = warmUpThreshold;
  }

  warmUpThreshold = op.getIntOption("trial-inlining-warmup-threshold");
  if (warmUpThreshold >= 0) {
    //jit::JitOptions.trialInliningWarmUpThreshold = warmUpThreshold;
  }

  warmUpThreshold = op.getIntOption("regexp-warmup-threshold");
  if (warmUpThreshold >= 0) {
    //jit::JitOptions.regexpWarmUpThreshold = warmUpThreshold;
  }

  if (op.getBoolOption("baseline-eager")) {
    //jit::JitOptions.setEagerBaselineCompilation();
  }

  if (op.getBoolOption("blinterp")) {
    //jit::JitOptions.baselineInterpreter = true;
  }

  if (op.getBoolOption("no-blinterp")) {
    //jit::JitOptions.baselineInterpreter = false;
  }

  if (op.getBoolOption("disable-jithints")) {
    //jit::JitOptions.disableJitHints = true;
  }

  if (op.getBoolOption("emit-interpreter-entry")) {
    //jit::JitOptions.emitInterpreterEntryTrampoline = true;
  }

  if (op.getBoolOption("no-emit-interpreter-entry")) {
    //jit::JitOptions.emitInterpreterEntryTrampoline = false;
  }

  warmUpThreshold = op.getIntOption("blinterp-warmup-threshold");
  if (warmUpThreshold >= 0) {
    //jit::JitOptions.baselineInterpreterWarmUpThreshold = warmUpThreshold;
  }

  if (op.getBoolOption("blinterp-eager")) {
    //jit::JitOptions.baselineInterpreterWarmUpThreshold = 0;
  }

  if (op.getBoolOption("no-baseline")) {
    //jit::JitOptions.baselineJit = false;
  }

  if (op.getBoolOption("no-ion")) {
    //jit::JitOptions.ion = false;
  }

  if (op.getBoolOption("no-native-regexp")) {
    //jit::JitOptions.nativeRegExp = false;
  }

  if (op.getBoolOption("trace-regexp-parser")) {
    //jit::JitOptions.trace_regexp_parser = true;
  }
  if (op.getBoolOption("trace-regexp-assembler")) {
    //jit::JitOptions.trace_regexp_assembler = true;
  }
  if (op.getBoolOption("trace-regexp-interpreter")) {
    //jit::JitOptions.trace_regexp_bytecodes = true;
  }
  if (op.getBoolOption("trace-regexp-peephole")) {
    //jit::JitOptions.trace_regexp_peephole_optimization = true;
  }

  if (op.getBoolOption("less-debug-code")) {
    //jit::JitOptions.lessDebugCode = true;
  }

  int32_t inliningEntryThreshold = op.getIntOption("inlining-entry-threshold");
  if (inliningEntryThreshold > 0) {
    //jit::JitOptions.inliningEntryThreshold = inliningEntryThreshold;
  }

  int32_t smallFunctionLength = op.getIntOption("small-function-length");
  if (smallFunctionLength > 0) {
    //jit::JitOptions.smallFunctionMaxBytecodeLength = smallFunctionLength;
  }

  if (const char* str = op.getStringOption("ion-regalloc")) {
    //jit::JitOptions.forcedRegisterAllocator = jit::LookupRegisterAllocator(str);
    //if (!jit::JitOptions.forcedRegisterAllocator.isSome()) {
    //  return OptionFailure("ion-regalloc", str);
    //}
  }

  if (op.getBoolOption("ion-eager")) {
    //jit::JitOptions.setEagerIonCompilation();
  }

  offthreadCompilation = true;
  if (const char* str = op.getStringOption("ion-offthread-compile")) {
    if (strcmp(str, "off") == 0) {
      offthreadCompilation = false;
    } else if (strcmp(str, "on") != 0) {
      return OptionFailure("ion-offthread-compile", str);
    }
  }
  //cx->runtime()->setOffthreadIonCompilationEnabled(offthreadCompilation);

  if (op.getStringOption("ion-parallel-compile")) {
    fprintf(stderr,
            "--ion-parallel-compile is deprecated. Please use "
            "--ion-offthread-compile instead.\n");
    return false;
  }

  if (op.getBoolOption("disable-bailout-loop-check")) {
    //jit::JitOptions.disableBailoutLoopCheck = true;
  }

  if (op.getBoolOption("enable-watchtower")) {
    //jit::JitOptions.enableWatchtowerMegamorphic = true;
  }
  if (op.getBoolOption("disable-watchtower")) {
    //jit::JitOptions.enableWatchtowerMegamorphic = false;
  }
  if (op.getBoolOption("only-inline-selfhosted")) {
    //jit::JitOptions.onlyInlineSelfHosted = true;
  }

  if (op.getBoolOption("enable-ic-frame-pointers")) {
    //jit::JitOptions.enableICFramePointers = true;
  }

  if (const char* str = op.getStringOption("ion-iterator-indices")) {
    if (strcmp(str, "on") == 0) {
      //jit::JitOptions.disableIteratorIndices = false;
    } else if (strcmp(str, "off") == 0) {
      //jit::JitOptions.disableIteratorIndices = true;
    } else {
      return OptionFailure("ion-iterator-indices", str);
    }
  }
  return true;
}

bool SetContextGCOptions(JSContext* cx, const OptionParser& op) {
  JS_SetGCParameter(cx, JSGC_MAX_BYTES, 0xffffffff);

  size_t nurseryBytes = op.getIntOption("nursery-size") * 1024L * 1024L;
  if (nurseryBytes == 0) {
    fprintf(stderr, "Error: --nursery-size parameter must be non-zero.\n");
    fprintf(stderr,
            "The nursery can be disabled by passing the --no-ggc option.\n");
    return false;
  }
  //JS_SetGCParameter(cx, JSGC_MAX_NURSERY_BYTES, nurseryBytes);

  size_t availMemMB = op.getIntOption("available-memory");
  if (availMemMB > 0) {
    //JS_SetGCParametersBasedOnAvailableMemory(cx, availMemMB);
  }

  if (const char* opt = op.getStringOption("nursery-strings")) {
    if (strcmp(opt, "on") == 0) {
      //cx->runtime()->gc.nursery().enableStrings();
    } else if (strcmp(opt, "off") == 0) {
      //cx->runtime()->gc.nursery().disableStrings();
    } else {
      MOZ_CRASH("invalid option value for --nursery-strings, must be on/off");
    }
  }

  if (const char* opt = op.getStringOption("nursery-bigints")) {
    if (strcmp(opt, "on") == 0) {
      //cx->runtime()->gc.nursery().enableBigInts();
    } else if (strcmp(opt, "off") == 0) {
      //cx->runtime()->gc.nursery().disableBigInts();
    } else {
      MOZ_CRASH("invalid option value for --nursery-bigints, must be on/off");
    }
  }

  bool incrementalGC = !op.getBoolOption("no-incremental-gc");
  JS_SetGCParameter(cx, JSGC_INCREMENTAL_GC_ENABLED, incrementalGC);

  if (op.getBoolOption("enable-parallel-marking")) {
    JS_SetGCParameter(cx, JSGC_PARALLEL_MARKING_ENABLED, true);
  }
  int32_t markingThreads = op.getIntOption("marking-threads");
  if (markingThreads > 0) {
    JS_SetGCParameter(cx, JSGC_MARKING_THREAD_COUNT, markingThreads);
  }

  JS_SetGCParameter(cx, JSGC_SLICE_TIME_BUDGET_MS, 5);

  JS_SetGCParameter(cx, JSGC_PER_ZONE_GC_ENABLED, true);

  for (MultiStringRange args = op.getMultiStringOption("gc-param");
       !args.empty(); args.popFront()) {
    //if (!SetGCParameterFromArg(cx, args.front())) {
    //  return false;
    //}
  }

#ifdef DEBUG
  dumpEntrainedVariables = op.getBoolOption("dump-entrained-variables");
#endif

#ifdef JS_GC_ZEAL
  const char* zealStr = op.getStringOption("gc-zeal");
  if (zealStr) {
    //if (!cx->runtime()->gc.parseAndSetZeal(zealStr)) {
    //  return false;
    //}
    uint32_t nextScheduled;
    //cx->runtime()->gc.getZealBits(&gZealBits, &gZealFrequency, &nextScheduled);
  }
#endif

  return true;
}
