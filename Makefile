asm = nasm

# 1. Define source files
BOOT_SOURCE = $(wildcard boot/*.asm)

all: build/boot.bin

# 2. Build bootloader
build/boot.bin: $(BOOT_SOURCE)
	$(asm) -f bin boot/boot.asm -o $@
