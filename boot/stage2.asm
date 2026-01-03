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
        
        jmp HALT
        
    
HALT:
    jmp $


protected_mode db "In Protected Mode", 0

times (2*512)-($-$$) db 0
