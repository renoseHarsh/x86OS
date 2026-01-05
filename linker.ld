ENTRY(kmain)

SECTIONS
{
    . = 0x100000;
	_kernel_start = .;

    /* Code section */
    .text ALIGN(0x1000) : {
        *(.text)
    }

	/* Read-only data section */
	.rodata ALIGN(0x1000) : {
		*(.rodata*)
	}

    /* Initialized data section */
    .data ALIGN(0x1000) : {
        *(.data)
    }

    /* Uninitialized data section (bss) */
    .bss ALIGN(0x1000) : {
        *(COMMON)
        *(.bss)
    }
	. = ALIGN(0x1000);
	/* End marker */
	_kernel_end = .;
}
