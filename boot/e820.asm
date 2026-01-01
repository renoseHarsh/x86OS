load_mmap:
    ; We store the map at 0x8000, immedieately above our 2nd stage loader
    mmap_n equ 0x8000           ; [4 bytes] Number of memory map entries
    mmap_ent equ 0x8000 + 4     ; [24 bytes each] Start of memory map entries
    SMAP equ 0x534D4150         ; 'SMAP' signature required by BIOS

    mov eax, 0xE820
    mov edx, SMAP
    xor ebx, 0              ; Continuation value at 0 for first call
    mov ecx, 24             ; Request 24 bytes ACPI 3.0 entry
    mov di, mmap_ent        ; ES:DI = buffer destination
    xor si, si              ; Entry count

    .loop:
        int 0x15
        jc .done            ; BIOS signals end of list via carry flag

        inc si

        ; Prepare registers for next iteration (BIOS often clobbers these) 
        mov eax, 0xE820
        mov edx, SMAP
        mov ecx, 24
        add di, 24          ; Advance buffer pointer for next entry
        

        test ebx, ebx       ; If ebx is 0, list is finished
        jnz .loop

    .done:
        mov [mmap_n], si   ; Save count for the kernel to read later
        ret
