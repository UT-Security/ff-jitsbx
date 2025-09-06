.text

.global monkeycage_cbtrampoline
monkeycage_cbtrampoline:
        // Entrypoint for callbacks.
        //Callback target is in %r10.
        //Callback index is in %rax.
        movq monkeycage_last_callback_invoked@gottpoff(%rip), %r11
        movq %rax, %fs:(%r11)
        jmpq *%r10
