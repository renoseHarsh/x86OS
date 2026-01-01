; -----------------------------------------------------------------------------
; print_message
; Purpose: Prints a null-terminated string to the screen via BIOS teletype.
; Input:   SI = Pointer to the string
; -----------------------------------------------------------------------------
print_message:
    pusha

    cld                     ; clear direction flag for string operations
    mov ah, 0x0E            ; teletype output function

    .loop:
        lodsb               ; AL = [DS:SI], SI++
        cmp al, 0           ; check for null terminator (\0)
        je .done
        int 0x10
        jmp .loop

    .done:
        call print_next_line
        popa
        ret

; -----------------------------------------------------------------------------
; print_next_line
; Purpose: Sends CR/LF to move the BIOS cursor to the start of the next row.
; -----------------------------------------------------------------------------
print_next_line:
    pusha
    mov ah, 0x0E
    mov al, 0x0D    ; ASCII Carriage Return: Move to col 0
    int 0x10
    mov al, 0x0A    ; ASCII Line Feed: Move to next row
    int 0x10
    popa
    ret
