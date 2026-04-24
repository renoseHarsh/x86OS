ENTRY(main)
PHDRS {
    rx PT_LOAD FLAGS(5);   /* r-x: read + execute */
    ro PT_LOAD FLAGS(4);   /* r--: read only */
    rw PT_LOAD FLAGS(6);   /* rw-: read + write */
}


SECTIONS {
    . = 0x40000000;
    .text ALIGN(0x1000) : {
        *(.text*)
    } :rx
    .rodata ALIGN(0x1000) : {
        *(.rodata*)
    } :ro
    .data ALIGN(0x1000) : {
        *(.data)
    } :rw
    .bss ALIGN(0x1000) : {
        *(COMMON)
        *(.bss)
    } :rw
}
