ENTRY(_start)

SECTIONS
{
    . = 0xC0100000;
	_kernel_start = .;

    _ro_start = .;
    /* Code section */
    .text ALIGN(0x1000) : {
        *(.text)
    }

	/* Read-only data section */
	.rodata ALIGN(0x1000) : {
		*(.rodata*)
	}
    . = ALIGN(0x1000);
    _ro_end = .;

    _rw_start = .;
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
    _rw_end = .;

	/* End marker */
	_kernel_end = .;
}
