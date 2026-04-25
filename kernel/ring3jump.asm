global jump_usermode
jump_usermode:
    mov ebx, [esp+4] ; arg 1 esp

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, ebx

    iret
