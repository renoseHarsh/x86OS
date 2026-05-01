global launch_process
section .text

%define USER_DATA_SEGMENT 0x23

; void launch_process();
launch_process:

    mov ax, USER_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    iret
