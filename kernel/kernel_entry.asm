section .multiboot_header
align 8
header_start:
    dd 0xE85250D6 
    dd 0
    dd header_end - header_start
    dd -(0xE85250D6 + 0 + (header_end - header_start))

    align 8
    dw 0
    dw 0
    dd 8
header_end:

[bits 32]
extern test
global _start
extern kmain
extern init_gdt

; Constants
KERNEL_VIRTUAL_ADDR equ 0xC0000000
PAGE_DIR_IDX equ KERNEL_VIRTUAL_ADDR >> 22

section .data
align 4096
page_directory:
    dd 0x00000083                           ; Map the first 4mb to the real first 4mb

    times (PAGE_DIR_IDX - 1) dd 0           ; No mapping till 3GB

    dd 0x00000083                           ; Map 3gb+4mb to the real first 4mb

    times (1024 - PAGE_DIR_IDX - 1) dd 0    ; No mapping from 3gb+4mb to 4gb


section .text
_start:

    mov esi, eax
    mov edi, ebx

    ; 1. Load the page directory into CR3
    mov ecx, (page_directory - KERNEL_VIRTUAL_ADDR)
    mov cr3, ecx

    ; 2. Enable PSE by setting the PSE bit in CR4, so we can use 4MB pages
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; 3. Enable paging and Write protect by setting the PG and WP bit in CR0
    mov eax, cr0
    or eax, 0x80010000 
    mov cr0, eax

    ; 4. Jump to higher half
    lea eax, [HigherHalf]
    jmp eax

HigherHalf:

    mov esp, stack_top
    mov ebp, esp

    ; 5. Update the GDT segment selectors
    call init_gdt

    ; 6. Unmap the first page
    mov dword [page_directory], 0x0
    invlpg [0]

    push edi
    push esi
    call kmain
    add esp, 8

    .hang:
        hlt
        jmp .hang


; Put the stack in hgiher half, save 16kib for it
; Allign it to 16 bytes, since GCC needs esp to be alligned to 16 bytes before calling functions
section .bss
align 0x10
    resb 0x4000
stack_top:
