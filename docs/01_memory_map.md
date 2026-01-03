# Memory Map

The memory map helps to understand how memory is organized in the system. It defines the different regions of memory and their purposes, and if they are available for use by the operating system or reserved for specific functions.

To retrive a memory map, we can use the BIOS interrupt 0x15 with function 0xE820. This funcZZtion returns a list of memory regions along with their types (available, reserved, ACPI reclaimable, etc.).

`boot/e820.asm` contains the code to retrieve and print the memory map using BIOS interrupt 0x15. It loads the memory map entried at `0x8000` which two sectors away from the boot sector loaded at `0x7C00`.

Why 0x8000?<br>
It's free memory area and not used by bootloader or stack (0x7C00 - 0x7E00), the next sector after 0x7C00 will be used by the second stage bootloader.


Reference:
- https://wiki.osdev.org/Memory_Map_(x86)
- https://wiki.osdev.org/Detecting_Memory_(x86)