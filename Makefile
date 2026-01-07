### Speicifies the tools to be used ###
ASM     := nasm
CC      := i686-elf-gcc
LD      := i686-elf-ld
OBJCOPY := i686-elf-objcopy
QEMU    := qemu-system-i386

# --- Project Structure ---
SRC_DIRS := kernel drivers hal lib

# --- Flags ---
INCLUDES := $(addprefix -I, $(SRC_DIRS))
CFLAGS   := -O0 -ffreestanding -m32 -g -c -Wall -Wextra $(INCLUDES)
LDFLAGS  := -T linker.ld -ffreestanding -nostdlib -lgcc -m32

# --- Search Path ---
VPATH := $(SRC_DIRS)

### All files needed for bootloader and kernel ###
BOOT_SOURCE     := $(wildcard boot/*.asm) 
KERNEL_SOURCE   := $(shell find . -type f -name '*.c')
KERNEL_HEADERS  := $(shell find . -type f -name '*.h') 
KERNEL_OBJ      := $(addprefix build/, $(notdir $(KERNEL_SOURCE:.c=.o)))
FINAL           := build/bootbable.bin

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

build/kernel.elf: build/kernel.o $(filter-out build/kernel.o, $(KERNEL_OBJ))
	$(CC) $(LDFLAGS) $^ -o $@

build/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*


run: all
	$(QEMU) -drive format=raw,file=$(FINAL)

debug: all
	$(QEMU) -drive format=raw,file=$(FINAL) -s -S & gdb "build/kernel.elf" -ex "target remote localhost:1234"; kill $!
