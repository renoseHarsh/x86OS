org 0x7C00
BITS 16


start:

    ; Initialize segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax


    ; Stack grown downwards from bootloader start
    mov sp, 0x7C00
    mov bp, sp
    sti

    mov [BOOT_DRIVE], dl    ; Store boot drive number


    ; --- Video Mode Setup ---
    mov ah, 0x00
    mov al, 0x03        ; 80x25 text mode
    int 0x10

    call load_mmap      ; Query BIOS for memory map (E820)
    mov si, done_mmap_msg
    call print_message

    ; --- Load Second Stage ---
    ; Reading 1 sector from LBA 1 to 0x7E00 (immediately after bootloader)
    stage2_addr equ 0x7E00
    mov ah, 0x02
    mov al, 0x01
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [BOOT_DRIVE]
    mov bx, stage2_addr
    int 0x13
    jc read_error     
    mov si, read_success_msg
    call print_message

    call setup_pm       ; Loads GDT and prepares for protected mode switch

    ; --- Switch to Protected Mode ---
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp code_sel:stage2_addr    ; Jump and reload CS


halt:
    cli
    hlt                 ; halt the CPU
    jmp $

read_error:
    mov si, read_err_msg
    call print_message
    jmp halt

%include "boot/print.asm"
%include "boot/e820.asm"
%include "boot/pm_setup.asm"

BOOT_DRIVE db 0

done_mmap_msg db "Memory Map Loaded", 0
read_success_msg db "Second Stage Loaded", 0
read_err_msg db "Error reading from disk.", 0

times 510-($-$$) db 0
dw 0xAA55

%include "boot/stage2.asm"
