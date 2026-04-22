global jump_usermode
jump_usermode:
    mov ebx, [esp+4] ; arg 1 eip
    mov ecx, [esp+8] ; arg 2 esp

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23
    push ecx
    pushf
    push 0x1B
    push ebx

    iret
