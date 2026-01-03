setup_pm:
disable_interupts:
    ; --- Disble Interrupts and NMI ---
    cli                     ; Mask maskable interrupts
    in al, 0x70             ; Disbale NMI (Non-Maskable Interrupt)
    or al, 0x80             ; Bit 7 is the NMI mask bit
    out 0x70, al
    in al, 0x71             ; Dummy read to stabilize CMOS bus

a20_line:
    .check_a20:
        ; Attempt to check if A20 line is enabled via BIOS, in a real kernel we'd
        ; fall back to the keyboard controller or Fast A20 if this fails
        mov ax, 0x2402 
        int 0x15

        test al, al             ; Check if A20 is already enabled
        jnz .done_a20           

        mov ax, 0x2401          ; BIOS function to enable A20 line
        int 0x15
        
    .done_a20:
        mov si, enabled_a20
        call print_message

load_gdt:
    lgdt [GDT_DESCRIPTOR]    ; Load the GDT
    mov si, loaded_gdt
    call print_message

ret

; --- Global Descriptor Table ---
GDT_START:
    dq 0                    ; Null Descriptor (Required)

    ; Code: Base=0, Limit=4GB, Type=Exec/Read, DPL=0, 32-bit
    GDT_CODE_SEGMENT:
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10011010b            ; Access Byte: Present, Ring 0, Code Segment, Executable, Readable
    db 11001111b            ; Flags: Granularity, 32-bit, Limit High
    db 0x00                 ; Base (bits 24-31)

    ; Data: Base=0, Limit=4GB, Type=Read/Write, DPL=0, 32-bit
    GDT_DATA_SEGMENT:
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10010010b            ; Access Byte: Present, Ring 0, Data Segment, Writable
    db 11001111b            ; Flags: Granularity, 32-bit, Limit High
    db 0x00                 ; Base (bits 24-31)
GDT_END:

GDT_DESCRIPTOR:
    dw GDT_END - GDT_START - 1  ; Size
    dd GDT_START                ; Offset

; Segment Selectors
code_sel equ 0x08                ; Offset 8, TI 0, RPL 0
data_sel equ 0x10                ; Offset 16, TI 0, RPL 0
enabled_a20 db "Enabled A20", 0
loaded_gdt db "Loaded GDT", 0
