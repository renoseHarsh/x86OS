asm = nasm

# 1. Define source files
BOOT_SOURCE = $(wildcard boot/*.asm)

all: build/boot.bin

# 2. Build bootloader
build/boot.bin: $(BOOT_SOURCE)
	mkdir -p build
	$(asm) -f bin boot/boot.asm -o $@ -l build/boot.lst
	truncate -s 1MB $@


run: all
	qemu-system-x86_64 -drive format=raw,file=build/boot.bin

debug: all
	qemu-system-x86_64 -drive format=raw,file=build/boot.bin -s -S & gdb -ex "target remote localhost:1234"; kill $!
clean:
	rm -rf build/*.bin build/*.lst

