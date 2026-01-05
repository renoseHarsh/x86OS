# Kernel Entered

The ATA PIO was able to load the kernel, but we were missing the kernel.

We made a kernel/kernel.c file which contains a simple kernel entry kmain function, that just loops forever.

This is what get's loaded by the kernel at 0x100000 (1MB) by the bootloader.

After loading we jump to that address.

### Additional Things needed to make this work
- Cross compiler to compile our code for the i386 architecture.
- Linker script which helps you place oarts of your code at specific memory addresses.
  - We created a `linker.ld` file for this.
- We needed to modify the Makefile to use the linker script and produce a flat binary file.
    - We compile and link the kernel code to an elf file using the linker script, this gives us debug symbols.
    - Then we use `objcopy` to convert the elf to a flat binary file.
    - We `cat` the bootloader binary and kernel binary together to make a single bootable image.
- Now cause the linker script cannot put the kmain fucntion at top of the kernel binary, we need to make sure that whenever we link all the object files together the file containing kmain `build/kernel.o` is the first file passed to the linker.

#### References
- https://wiki.osdev.org/Linker_Scripts
- https://wiki.osdev.org/GCC_Cross-Compiler