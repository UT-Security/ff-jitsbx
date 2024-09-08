/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jitsbx_JitSandboxMask_h 
#define jitsbx_JitSandboxMask_h 

#include <mutex>
#include <sys/stat.h>
#include <unistd.h>

#define SANDBOX_LOG(format, ...)                               \
      log_mutex.lock();                                        \
      fprintf(sandbox_log, format __VA_OPT__(, ) __VA_ARGS__); \
      fflush(sandbox_log);                                     \
      log_mutex.unlock();

namespace js {
  namespace sandbox {
    void checkJitMask(void* ptr);
    void init();
  }
}

#endif /* jitsbx_JitSandboxMask_h */

extern std::mutex log_mutex;
extern void js::sandbox::checkJitMask(void* ptr);
extern FILE* sandbox_log;
