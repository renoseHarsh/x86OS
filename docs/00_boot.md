# Booting

To boot the OS, the bios expects a boot sector on the boot device. The boot sector is the first sector (512 bytes) of the bootable device (like a hard drive, floppy disk, or USB drive). When the system is powered on, the BIOS loads this sector into memory at address `0x7C00` and transfers control to it.

The boot sector program typically performs the following tasks:
1. Sets up the CPU in real mode.
2. Initializes the stack.
3. Loads the kernel or a second stage bootloader into memory.
4. Transfers control to the loaded kernel or bootloader.

The boot sector must end with a specific signature (0x55AA) to be recognized as a valid boot sector by the BIOS. If the signature is not present, the BIOS will not attempt to boot from that device and will move on to the next boot device in the boot order.

Steps I did:
Created `boot/boot.asm` with boot sector code.
Crreated a `Makefile` for automated building.


Makefile produces a bootable flat binary `boot.bin` which qemu can boot from.

This boot file will be responsible for all the 4 steps mentioned above.

Refrence:
- https://wiki.osdev.org/Boot_Sequence