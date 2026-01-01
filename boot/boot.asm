org 0x7C00
BITS 16


start:
    cli
    xor ax, ax          ; clear ax register
    mov ds, ax          ; zero data segment
    mov es, ax          ; zero extra segment
    mov ss, ax          ; zero stack segment

    mov sp, 0x9000      ; initialize stack pointer
    mov bp, sp          ; initialize base pointer
    sti

    mov ah, 0x00
    mov al, 0x03
    int 0x10            ; set video mode 0x13 (320x200x256)

    mov si, start_boot      ; load address of boot message
    call print_message      ; print the boot message

    cli
    hlt                 ; halt the CPU
    jmp $

%include "boot/print.asm"


        

start_boot db "Booting...", 0
times 510-($-$$) db 0
dw 0xAA55
