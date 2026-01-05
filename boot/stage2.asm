BITS 32

INIT_PM:
    ; Initialize data segments
    mov ax, data_sel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Setup stack
    mov ebp, 0x80000
    mov esp, ebp            


PRINT:
    mov ebx, 0xB8000 + (4 * (80 * 2))       ; VGA text buffer
    mov esi, protected_mode
    .loop:
        lodsb
        cmp al, 0
        je .done

        mov [ebx], al                   ; Write character
        mov byte [ebx + 1], 0x07        ; Light green on black

        add ebx, 2                      ; Move to next character cell
        jmp .loop

    .done:
        ; Move cursor to next line
        mov ax, 5               ; Row 5
        mov bx, 0               ; Column 0

        mov dl, 80              ; Number of columns

        ; Compute offset = row * num_cols + col
        mul dl
        add bx, ax

        ; Activate High Byte Register
        mov dx, 0x03D4
        mov al, 0x0F
        out dx, al
        ; Move high byte to Data Register
        inc dl
        mov al, bl              
        out dx, al

        ; Activate Low Byte Register
        dec dl                  
        mov al, 0x0E
        out dx, al
        ; Move low byte to Data Register
        inc dl
        mov al, bh
        out dx, al
        
ATA_PIO_READ:
; Read main kernel sector from ATA bus using PIO mode

    xor ax, ax
    xor cx, cx
    ; Buffer to store kernel sector
    kernel_buffer equ 0x100000
    mov edi, kernel_buffer

    ; Define IO base for primary ATA bus
    IO_BASE equ 0x1F0


    ; Send LBA mode + Drive Select + bits 24-27 of lba to IO base
    mov dx, IO_BASE + 6
    mov al, 0xE0            ; 0xE0 for master drive + LBA mode + bits 24-27 = 0
    out dx, al

    ; Waste time (Send NULL byte to IO base + 1)
    mov dx, IO_BASE + 1
    mov al, 0x00
    out dx, al

    ; Send number of sectors to read (256 sector) to IO base + 2
    mov dx, IO_BASE + 2
    mov al, 0x00        ; 0 means 256 sectors
    mov ecx, 256        ; Save number of sectors to read in ecx
    out dx, al

    ; Send LBA low byte (bits 0-7) to IO base + 3
    mov dx, IO_BASE + 3
    mov al, 0x02       ; sector 2
    out dx, al

    ; Send LBA mid byte (bits 8-15) to IO base + 4
    mov dx, IO_BASE + 4
    mov al, 0x00
    out dx, al

    ; Send LBA high byte (bits 16-23) to IO base + 5
    mov dx, IO_BASE + 5
    mov al, 0x00
    out dx, al

    ; Send read command (0x20) to IO base + 7
    mov dx, IO_BASE + 7
    mov al, 0x20
    out dx, al


    .read_sector_loop:
        ; Loop for multiple sectors if needed
        push ecx        ; Save sector count

        ; Wait for BSY to clear and DRQ to set
        mov dx, IO_BASE + 7
        .wait_drq:
            in al, dx

            test al, 0x01       ; ERR bit
            jnz .error          ; Jump to error handling if ERR is set

            test al, 0x80       ; BSY bit
            jnz .wait_drq       ; Wait while BSY is set

            test al, 0x08       ; DRQ bit
            jz .wait_drq        ; Wait until DRQ is set

        ; Read 256 words (512 bytes) from data port
        mov dx, IO_BASE
        mov cx, 256             ; 256 words = 512 bytes
        rep insw                ; Read words into [edi]

        pop ecx         ; Restore sector count
        dec ecx
        test ecx, ecx
        jnz .read_sector_loop
        jmp .done

        .error:
            mov dx, IO_BASE + 1
            in al, dx
            jmp HALT

        .done:

    mov dx, IO_BASE + 7
    in al, dx


    jmp kernel_buffer

        
    
HALT:
    jmp $




protected_mode db "In Protected Mode", 0

times (2*512)-($-$$) db 0
