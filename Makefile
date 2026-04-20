### Speicifies the tools to be used ###
ASM     := nasm
CC      := i686-elf-gcc
GDB     := i686-elf-gdb
QEMU    := qemu-system-i386

# --- Project Structure ---
SRC_DIRS := kernel drivers lib mm interrupts sched gdt
INCLUDES := $(addprefix -I, $(SRC_DIRS))
 

# --- Flags ---
CFLAGS   := -O0 -ffreestanding -m32 -g -c -Wall -Wextra -Wno-unused $(INCLUDES)
LDFLAGS  := -T linker.ld -ffreestanding -nostdlib -lgcc -m32

# --- Files ---
ISO_NAME    	:= os.iso
ISO_DIR 		:= isodir
KERNEL_BIN      := $(ISO_DIR)/boot/kernel.elf
GRUB_CFG_SRC    := grub.cfg
GRUB_CFG_DST    := $(ISO_DIR)/boot/grub/grub.cfg

# Dynamically find sources
KERNEL_SOURCE   := $(shell find $(SRC_DIRS) -type f -name '*.c')
KERNEL_ASM      := $(shell find $(SRC_DIRS) -type f -name '*.asm')

# Map object files
KERNEL_OBJ      := $(patsubst %.c, build/%.o, $(KERNEL_SOURCE))
KERNEL_OBJ      += $(patsubst %.asm, build/%.o, $(KERNEL_ASM))

# --- Targets ---
.PHONY: all clean run debug

all: $(ISO_NAME)

$(ISO_NAME): $(KERNEL_BIN) $(GRUB_CFG_DST)
	i686-elf-grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_BIN): $(KERNEL_OBJ) | $(ISO_DIR)/boot
	$(CC) $^ $(LDFLAGS) -o $@

build/%.o: %.asm | build
	@mkdir -p $(dir $@)
	$(ASM) -g -f elf32 $< -o $@

build/%.o: %.c | build
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(GRUB_CFG_DST): $(GRUB_CFG_SRC) | $(ISO_DIR)/boot/grub
	cp $< $@

build $(ISO_DIR)/boot $(ISO_DIR)/boot/grub:
	mkdir -p $@

clean:
	rm -rf build $(ISO_DIR) $(ISO_NAME) serial.log

run: all
	$(QEMU) -cdrom $(ISO_NAME) -serial file:serial.log

debug: all
	$(QEMU) -cdrom $(ISO_NAME) -s -S & gdb "$(KERNEL_BIN)" -ex "target remote localhost:1234"; kill $$! 2>/dev/null || true
