org 0x7C00
BITS 16


start:
    cli
    xor ax, ax          ; clear ax register
    mov ds, ax          ; zero data segment
    mov es, ax          ; zero extra segment
    mov ss, ax          ; zero stack segment

    mov sp, 0x7C00      ; initialize stack pointer
    mov bp, sp          ; initialize base pointer
    sti

    mov ah, 0x00
    mov al, 0x03
    int 0x10            ; set video mode 0x13

    ; Print messages
    mov si, start_boot_msg
    call print_message
    mov si, load_mmap_msg
    call print_message

    call load_mmap      ; Load memory map
    mov si, done_mmap_msg
    call print_message


    jmp halt

halt:
    cli
    hlt                 ; halt the CPU
    jmp $

%include "boot/print.asm"
%include "boot/e820.asm"


        

start_boot_msg db "Booting...", 0
load_mmap_msg db "Loading memory map...", 0
done_mmap_msg db "Memory map loaded successfully.", 0
times 510-($-$$) db 0
dw 0xAA55
