/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: sw=2 ts=4 et :
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsXPCOM.h"
#include "nsXULAppAPI.h"
#include "mozilla/Bootstrap.h"
#include "XREChildData.h"

#ifdef XP_WIN
#  include <windows.h>
// we want a wmain entry point
#  include "nsWindowsWMain.cpp"
#else
// FIXME/cjones testing
#  include <unistd.h>
#endif

#if defined(XP_WIN) && defined(MOZ_SANDBOX)
#  include "mozilla/sandboxing/SandboxInitialization.h"
#  include "mozilla/sandboxing/sandboxLogging.h"
#endif

// test(JS_SANDBOX_CET)
#include <asm/prctl.h>   /* Definition of ARCH_* constants */
#include <sys/syscall.h> /* Definition of SYS_* constants */
#include <unistd.h>
#define ARCH_PRCTL(arg1, arg2)                           \
    ({                                                     \
      long _ret;                                           \
      register long _num __asm__("eax") = __NR_arch_prctl; \
      register long _arg1 __asm__("rdi") = (long)(arg1);   \
      register long _arg2 __asm__("rsi") = (long)(arg2);   \
                                                           \
      __asm__ volatile("syscall\n"                         \
                       : "=a"(_ret)                        \
                       : "r"(_arg1), "r"(_arg2), "0"(_num) \
                       : "rcx", "r11", "memory", "cc");    \
      _ret;                                                \
    })

#define ARCH_SHSTK_ENABLE 0x5001
#define ARCH_SHSTK_DISABLE 0x5002
#define ARCH_SHSTK_SHSTK (1ULL << 0)

#define ENABLE_SHSTK() ARCH_PRCTL(ARCH_SHSTK_ENABLE, ARCH_SHSTK_SHSTK)
#define DISABLE_SHSTK() ARCH_PRCTL(ARCH_SHSTK_DISABLE, ARCH_SHSTK_SHSTK)

int content_process_main(mozilla::Bootstrap* bootstrap, int argc,
                         char* argv[]) {
  // Check for the absolute minimum number of args we need to move
  // forward here. We expect the last arg to be the child process type.
  if (argc < 1) {
    return 3;
  }

  XREChildData childData;

#if defined(XP_WIN) && defined(MOZ_SANDBOX)
  if (IsSandboxedProcess()) {
    childData.sandboxTargetServices =
        mozilla::sandboxing::GetInitializedTargetServices();
    if (!childData.sandboxTargetServices) {
      return 1;
    }

    childData.ProvideLogFunction = mozilla::sandboxing::ProvideLogFunction;
  }
#endif

  bootstrap->XRE_SetProcessType(argv[--argc]);

#if defined(XP_WIN) && defined(MOZ_SANDBOX)
  if (bootstrap->XRE_GetProcessType() == GeckoProcessType_RemoteSandboxBroker) {
    childData.sandboxBrokerServices =
        mozilla::sandboxing::GetInitializedBrokerServices();
  }
#endif

  // test(JS_SANDBOX_CET)
  ENABLE_SHSTK();

  nsresult rv = bootstrap->XRE_InitChildProcess(argc, argv, &childData);

  // test(JS_SANDBOX_CET)
  DISABLE_SHSTK();
  return NS_FAILED(rv);
}
