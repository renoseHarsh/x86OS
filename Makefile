### Speicifies the tools to be used ###
ASM     := nasm
CC      := i686-elf-gcc
GDB     := i686-elf-gdb
QEMU    := qemu-system-i386

# --- Project Structure ---
SRC_DIRS  := kernel drivers lib mm interrupts sched gdt process
INCLUDES  := $(addprefix -I, $(SRC_DIRS))
USER_DIRS := user
 

# --- Flags ---
CFLAGS     := -O0 -ffreestanding -m32 -g -c -Wall -Wextra -Wno-unused $(INCLUDES)
LDFLAGS    := -T linker.ld -ffreestanding -nostdlib -lgcc -m32
USER_CFLAGS := -O0 -ffreestanding -m32 -c -Wall -Wextra -Wno-unused
USER_LDFLAGS := -T user.ld -ffreestanding -nostdlib -lgcc -m32

# --- Files ---
ISO_NAME    	:= os.iso
ISO_DIR 		:= isodir
KERNEL_ELF      := $(ISO_DIR)/boot/kernel.elf
USER_ELF        := $(ISO_DIR)/boot/user.elf
GRUB_CFG_SRC    := grub.cfg
GRUB_CFG_DST    := $(ISO_DIR)/boot/grub/grub.cfg

# Dynamically find sources
KERNEL_SOURCE   := $(shell find $(SRC_DIRS) -type f -name '*.c')
KERNEL_ASM      := $(shell find $(SRC_DIRS) -type f -name '*.asm')
USER_SOURCE     := $(shell find $(USER_DIRS) -type f -name '*.c')
USER_ASM        := $(shell find $(USER_DIRS) -type f -name '*.asm')

# Map object files
KERNEL_OBJ      := $(patsubst %.c, build/%.o, $(KERNEL_SOURCE))
KERNEL_OBJ      += $(patsubst %.asm, build/%.o, $(KERNEL_ASM))
USER_C_OBJ      := $(patsubst %.c, build/%.o, $(USER_SOURCE))
USER_ASM_OBJ    := $(patsubst %.asm, build/%.o, $(USER_ASM))


# --- Targets ---
.PHONY: all clean run debug

all: $(ISO_NAME)

$(ISO_NAME): $(KERNEL_ELF) $(USER_ELF) $(GRUB_CFG_DST)
	i686-elf-grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_ELF): $(KERNEL_OBJ) | $(ISO_DIR)/boot
	$(CC) $^ $(LDFLAGS) -o $@

build/%.o: %.asm | build
	@mkdir -p $(dir $@)
	$(ASM) -g -f elf32 $< -o $@

build/%.o: %.c | build
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(USER_ELF): $(USER_ASM_OBJ) $(USER_C_OBJ) | $(ISO_DIR)/boot
	$(CC) $^ $(USER_LDFLAGS) -o $@

$(USER_C_OBJ): build/$(USER_DIRS)/%.o: $(USER_DIRS)/%.c | build
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) $< -o $@

$(USER_ASM_OBJ): build/$(USER_DIRS)/%.o: $(USER_DIRS)/%.asm | build
	@mkdir -p $(dir $@)
	$(ASM) -g -f elf32 $< -o $@

$(GRUB_CFG_DST): $(GRUB_CFG_SRC) | $(ISO_DIR)/boot/grub
	cp $< $@

build $(ISO_DIR)/boot $(ISO_DIR)/boot/grub:
	mkdir -p $@

clean:
	rm -rf build $(ISO_DIR) $(ISO_NAME) serial.log

run: all
	$(QEMU) -cdrom $(ISO_NAME) -display none -serial stdio

debug: all
	$(QEMU) -cdrom $(ISO_NAME) -serial file:serial.log -s -S & \
	gdb "$(KERNEL_ELF)" \
		-ex "target remote localhost:1234" \
		-ex "set confirm off" \
		-ex "add-symbol-file $(USER_ELF)" \
		-ex "set confirm on"; \
	kill $$! 2>/dev/null || true
