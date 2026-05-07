global exit
global sleep

section .text

; void exit();
exit:
    mov eax, 0
    int 0x80
    ret

; void sleep(uint32_t time);
sleep:
    push ebx

    mov ebx, [esp + 8]
    mov eax, 1
    int 0x80

    pop ebx
    ret
