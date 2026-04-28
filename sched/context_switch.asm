global context_switch
section .text

; void context_switch(uint32 *prev_esp, uint32 cur_esp);
context_switch:
    mov ecx, [esp + 4] ; old esp pointer arg1
    mov edx, [esp + 8] ; new esp arg2

    pusha
    mov [ecx], esp ; Save esp in current thread
    mov esp, edx ; use next thread's esp
    popa

    ret
