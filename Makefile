# Speicifies the tools to be used
ASM := nasm
CC := i686-elf-gcc
LD := i686-elf-ld
OBJCOPY := i686-elf-objcopy
QEMU := qemu-system-i386

# Defines Flags
CFLAGS := -O0 -ffreestanding -m32 -g -c -Wall -Wextra
# No optimizations, freestanding environment, 32-bit mode, debug info, compile only, all warnings
LDFLAGS := -T linker.ld -ffreestanding -nostdlib -lgcc -m32


# All files needed for bootloader
BOOT_SOURCE = $(wildcard boot/*.asm)


all: build/bootable.bin

# Merge bootloader and kernel into a single bootable binary, truncate so ATA PIO doesn't hang
build/bootable.bin: build/boot.bin build/kernel.bin
	cat $^ > $@
	truncate -s 1MB $@

# Assemble bootloader source files into binary format
build/boot.bin: $(BOOT_SOURCE)
	mkdir -p build
	$(ASM) -f bin boot/boot.asm -o $@

# Convert ELF kernel to binary format
build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@

# Compile and link kernel source files into ELF format
build/kernel.elf: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o build/kernel.o
	$(CC) $(LDFLAGS) build/kernel.o -o $@

	

clean:
	rm -rf build/*

run: all
	$(QEMU) -drive format=raw,file=build/bootable.bin

debug: all
	$(QEMU) -drive format=raw,file=build/bootable.bin -s -S & gdb "build/kernel.elf" -ex "target remote localhost:1234"; kill $!
