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

    ; Print messages
    mov si, start_boot_msg
    call print_message
    mov si, load_mmap_msg
    call print_message

    call load_mmap      ; Load memory map
    mov si, done_mmap_msg
    call print_message

    ; Read second sector into memory at 0x7E00
    mov si, read_msg
    call print_message

    mov ah, 0x02        ; BIOS read sectors function
    mov al, 0x01        ; number of sectors to read
    mov ch, 0x00        ; cylinder 0
    mov cl, 0x02        ; sector 2
    mov dh, 0x00        ; head 0
    mov dl, 0x80        ; drive 0x80 (first hard disk)
    mov bx, 0x7E00      ; buffer to read into
    int 0x13            ; call BIOS disk interrupt

    jc read_error      ; jump if carry flag is set (error)

    mov si, read_success_msg
    call print_message


    jmp halt

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

BOOT_DRIVE db 0

start_boot_msg db "Booting...", 0
load_mmap_msg db "Loading memory map...", 0
done_mmap_msg db "Memory map loaded successfully.", 0
read_msg db "Reading second stage from disk...", 0
read_success_msg db "Second stage loaded successfully.", 0
read_err_msg db "Error reading from disk.", 0

times 510-($-$$) db 0
dw 0xAA55

%include "boot/stage2.asm"
xor ax, ax
