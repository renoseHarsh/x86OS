org 0x7C00
BITS 16


start:
    mov [BOOT_DRIVE], dl    ; Store boot drive number

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

    call load_mmap      ; Load memory map

    mov si, done_mmap_msg
    call print_message

    stage2_addr equ 0x7E00

    ; Read second sector into memory at 0x7E00
    mov ah, 0x02            ; BIOS read sectors function
    mov al, 0x01            ; number of sectors to read
    mov ch, 0x00            ; cylinder 0
    mov cl, 0x02            ; sector 2
    mov dh, 0x00            ; head 0
    mov dl, 0x80            ; drive 0x80 (first hard disk)
    mov bx, stage2_addr     ; buffer to read into
    int 0x13                ; call BIOS disk interrupt

    jc read_error      ; jump if carry flag is set (error)

    mov si, read_success_msg
    call print_message

    call setup_pm    ; Setup protected mode

    mov eax, cr0        ; Get control register 0
    or eax, 1           ; Set the PE (Protection Enable) bit
    mov cr0, eax        ; Update control register 0

    jmp code_sel:stage2_addr    ; Far jump to protected mode code segment


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
xor ax, ax
