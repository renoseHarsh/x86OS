extern interrupt_handler

isr_wrapper:
    cld

    pusha

    push esp        ; Pass pointer to stack arguments
    call interrupt_handler
    add esp, 4      ; Drop the pointer argument

    popa
    
    add esp, 8      ; Drop error code and interrupt number

    iret


; ISR stubs for interrupts with and without error codes
%macro isr_err_stub 1
isr_stub_%+%1:
    push %1                     ; Push interrupt number
    jmp isr_wrapper
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0                      ; Push dummy error code
    push %1                     ; Push interrupt number
    jmp isr_wrapper
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_err_stub    21
%assign i 22
%rep 234
    isr_no_err_stub i
%assign i i+1
%endrep


global isr_stub_table
isr_stub_table:
    %assign i 0
    %rep 256
        dd isr_stub_%+i
        %assign i i + 1
    %endrep
