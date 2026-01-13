[bits 32]
global _start
extern kmain

; Constants
KERNE_VIRTUAL_ADDR equ 0xC0000000
PAGE_DIR_IDX equ KERNE_VIRTUAL_ADDR >> 22

section .data
align 4096
page_directory:
    dd 0x00000083                           ; Map the first 4mb to the real first 4mb

    times (PAGE_DIR_IDX - 1) dd 0           ; No mapping till 3GB

    dd 0x00000083                           ; Map 3gb+4mb to the real first 4mb

    times (1024 - PAGE_DIR_IDX - 1) dd 0    ; No mapping from 3gb+4mb to 4gb


section .text
_start:
    ; 1. Load the page directory into CR3
    mov ecx, (page_directory - KERNE_VIRTUAL_ADDR)
    mov cr3, ecx

    ; 2. Enable PSE by setting the PSE bit in CR4, so we can use 4MB pages
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; 3. Enable paging by setting the PG bit in CR0
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; 4. Jump to higher half
    lea eax, [HigherHalf]
    jmp eax

HigherHalf:

    mov esp, stack_top
    mov ebp, esp

    ; 5. Update the GDT segment selectors
    sub esp, 6
    sgdt [esp]                      ; Store current GDT descriptor
    mov eax, [esp + 2]              ; Extract the base address
    add eax, KERNE_VIRTUAL_ADDR
    mov [esp + 2], eax
    lgdt [esp]                      

    ; 6. Unmap the first page
    mov dword [page_directory], 0x0
    invlpg [0]

    call kmain

    cli
    hlt

; Put the stack in hgiher half, save 16kib for it
; Allign it to 16 bytes, since GCC needs esp to be alligned to 16 bytes before calling functions
section .bss
align 0x10
resb 0x4000
stack_top:
