.section .rodata
.global rlbox_lfi_start
.global rlbox_lfi_end
rlbox_lfi_start:
    .incbin "rlbox.lfi"
rlbox_lfi_end:

.section .note.GNU-stack,"",@progbits
