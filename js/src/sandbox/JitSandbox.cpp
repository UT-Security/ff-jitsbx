// ask2374
#include "sandbox/JitSandbox.h"

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

  void* gsbase;
  __asm__ __volatile__("rdgsbase %0" : "=r"(gsbase));
  void* masked_ptr = (void*)(((uint64_t)ptr) & 0xFFFFFFFF00000000);

  u_int64_t rsp;
  __asm__ __volatile__("movq %%rsp,%0" : "=r"(rsp));
  int set = rsp & 0xf;
  if (set) {
    __asm__ __volatile__("push %rax\n");
  }
  if (masked_ptr == gsbase) {
    SANDBOX_LOG("checkJitMask,pass,%p\n", ptr);
  } else {
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
// ask2374
