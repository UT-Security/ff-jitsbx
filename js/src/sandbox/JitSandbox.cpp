// ask2374
#include "sandbox/JitSandbox.h"

void js::sandbox::checkJitMask(void* ptr) {
	__asm__ __volatile__ (
			"push rbx\n"
			"push rdi\n"
			"push rsi\n"
			"push r12\n"
			"push r13\n"
			"push r14\n"
			"push r15\n"
	);

	SANDBOX_LOG("checkJitMask,%p\n", ptr);

	__asm__ __volatile__ (
			"pop r15\n"
			"pop r14\n"
			"pop r13\n"
			"pop r12\n"
			"pop rsi\n"
			"pop rdi\n"
			"pop rbx\n"
	);
}
// ask2374
