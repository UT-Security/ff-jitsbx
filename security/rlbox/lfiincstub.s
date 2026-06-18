.section .rodata
.global rlbox_lfi_start
.global rlbox_lfi_end
rlbox_lfi_start:
    .incbin "REPLACE_ME_PATH"
rlbox_lfi_end:

.section .note.GNU-stack,"",@progbits
