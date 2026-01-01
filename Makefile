asm = nasm

all: boot.bin

boot.bin: boot/boot.asm
	$(asm) -f bin $^ -o $@
