asm = nasm

all: build/boot.bin

build/boot.bin: boot/boot.asm boot/print.asm boot/e820.asm
	$(asm) -f bin $< -o $@
