### Speicifies the tools to be used ###
ASM		:= nasm
CC		:= i686-elf-gcc
LD		:= i686-elf-ld
OBJCOPY	:= i686-elf-objcopy
QEMU 	:= qemu-system-i386

### Defines Flags ###
CFLAGS		:= -O0 -ffreestanding -m32 -g -c -Wall -Wextra
LDFLAGS		:= -T linker.ld -ffreestanding -nostdlib -lgcc -m32

FINAL	:= build/bootable.bin


### All files needed for bootloader and kernel ###
BOOT_SOURCE		:= $(wildcard boot/*.asm) 
KERNEL_SOURCE	:= $(wildcard kernel/*.c)
KERNEL_HEADERS	:= $(wildcard kernel/*.h)
KERNEL_OBJ 		:= $(addprefix build/, $(notdir $(KERNEL_SOURCE:.c=.o)))

### Makefile Rules ###
all: $(FINAL) 

$(FINAL): build/boot.bin build/kernel.bin
	cat $^ > $@
	truncate -s 1MB $@

build/boot.bin: $(BOOT_SOURCE)
	mkdir -p build
	$(ASM) -f bin boot/boot.asm -o $@ -l build/boot.lst 

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@

build/kernel.elf: $(KERNEL_OBJ)
	$(CC) $(LDFLAGS) $(KERNEL_OBJ) -o $@

build/%.o: kernel/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*

run: all
	$(QEMU) -drive format=raw,file=$(FINAL)

debug: all
	$(QEMU) -drive format=raw,file=$(FINAL) -s -S & gdb "build/kernel.elf" -ex "target remote localhost:1234"; kill $!
