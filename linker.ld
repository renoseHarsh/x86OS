ENTRY(_start)

SECTIONS
{
    . = 0xC0100000;

    _ro_start = .;
    _kernel_start = .;
    .multiboot_header ALIGN(8) : AT(ADDR(.multiboot_header)-0xC0000000) {
        *(.multiboot_header)
    }

    /* Code section */
    .text ALIGN(0x1000) : AT(ADDR(.text)-0xC0000000) {
        *(.text)
    }

	/* Read-only data section */
	.rodata ALIGN(0x1000) : AT(ADDR(.rodata)-0xC0000000) {
		*(.rodata*)
	}
    . = ALIGN(0x1000);
    _ro_end = .;

    _rw_start = .;
    /* Initialized data section */
    .data ALIGN(0x1000) : AT(ADDR(.data)-0xC0000000) {
        *(.data)
    }

    /* Uninitialized data section (bss) */
    .bss ALIGN(0x1000) : AT(ADDR(.bss)-0xC0000000) {
        *(COMMON)
        *(.bss)
    }

	. = ALIGN(0x1000);
    _rw_end = .;

	/* End marker */
	_kernel_end = .;
}
