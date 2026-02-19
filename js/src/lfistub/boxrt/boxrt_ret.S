.section ".text.trusted"

#if defined(__aarch64__)
.p2align 5
.global _lfi_ret
_lfi_ret:
        ldr x30, [x27, #24]
        blr x30
#elif defined(__x86_64__)
.p2align 5
.global _lfi_ret
_lfi_ret:
	lea 0x4(%rip), %r11
	jmpq *0x18(%r14)
#else
#error "invalid architecture"
#endif
