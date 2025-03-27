/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jitsbx/JitSandboxMask.h"

#include <iostream>
void js::sandbox::init() {
  __asm__ __volatile__("wrgsbase %0": : "r" (0x100000000));
#ifdef JITSBX_HEAP_MASK
  char log_file_path[100];
  snprintf(log_file_path, 100, "/tmp/mozilla/%d.log", getpid());
  log_mutex.lock();
  char tmp_directory[] = "/tmp/mozilla/";
  struct stat st = {0};
  if (stat(tmp_directory, &st) == -1) {
    mkdir(tmp_directory, 0700);
  }
  sandbox_log = fopen(log_file_path, "w");
  log_mutex.unlock();
#endif

}

void js::sandbox::checkJitMask(void* ptr) {
  __asm__ __volatile__(
      "push %rax\n"
      "push %rcx\n"
      "push %rdx\n"
      "push %rbx\n"
      "push %rsi\n"
      "push %rdi\n"
      "push %r8\n"
      "push %r9\n"
      "push %r10\n"
      "push %r11\n"
      "push %r12\n"
      "push %r13\n"
      "push %r14\n"
      "push %r15\n");

  u_int64_t rsp;
  __asm__ __volatile__("movq %%rsp,%0" : "=r"(rsp));
  int set = rsp & 0xf;
  if (set) {
    __asm__ __volatile__("push %rax\n");
  }

  void* gsbase;
  __asm__ __volatile__("rdgsbase %0" : "=r"(gsbase));
  void* masked_ptr = (void*)(((uint64_t)ptr) & 0xFFFFFFFF00000000);

  if (masked_ptr != gsbase) {
    SANDBOX_LOG("checkJitMask,fail,%p,%p\n", gsbase, ptr);
  }

  if (set) {
    __asm__ __volatile__("pop %rax\n");
  }

  __asm__ __volatile__(
      "pop %r15\n"
      "pop %r14\n"
      "pop %r13\n"
      "pop %r12\n"
      "pop %r11\n"
      "pop %r10\n"
      "pop %r9\n"
      "pop %r8\n"
      "pop %rdi\n"
      "pop %rsi\n"
      "pop %rbx\n"
      "pop %rdx\n"
      "pop %rcx\n"
      "pop %rax\n");
}
