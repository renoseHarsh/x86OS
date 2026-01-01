asm = nasm

all: boot.bin

boot.bin: boot/boot.asm boot/print.asm boot/e820.asm
	$(asm) -f bin $< -o $@
