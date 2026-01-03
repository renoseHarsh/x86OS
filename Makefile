asm = nasm

# 1. Define source files
BOOT_SOURCE = $(wildcard boot/*.asm)

all: build/boot.bin
	mkdir -p build

# 2. Build bootloader
build/boot.bin: $(BOOT_SOURCE)
	$(asm) -f bin boot/boot.asm -o $@ -l build/boot.lst

run: all
	qemu-system-x86_64 -drive format=raw,file=build/boot.bin

debug: all
	qemu-system-x86_64 -drive format=raw,file=build/boot.bin -s -S

clean:
	rm -rf build/*.bin build/*.lst

