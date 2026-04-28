global context_switch

section .text

; void context_switch(uint32_t *old_esp, uint32_t new_esp)
context_switch:
    mov ecx, [esp + 4] ; old esp arg 1
    mov edx, [esp + 8] ; new esp arg 2

    pusha
    mov [ecx], esp
    mov esp, edx
    popa

    ret
