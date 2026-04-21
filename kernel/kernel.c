#include "gdt.h"
#include "idt.h"
#include "kprintf.h"
#include "layout.h"
#include "panic.h"
#include "pic.h"
#include "pit.h"
#include "sched.h"
#include "serial.h"
#include "vga.h"
#include "vmm.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t size;
    uint32_t reserved;
} __attribute__((packed)) header_t;

typedef struct {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    memory_map_t entries[0];
} multiboot_tag_mmap;

memory_map_t memory_map[6];

void kmain(uint32_t magic, uint32_t mbi_ptr)
{
    seriel_init();
    vga_clear();
    if (magic != 0x36d76289) {
        kprintf("FATAL: Invalid Multiboot2 magic number: 0x%x\n", magic);
        kernel_panic();
    }

    header_t *header = (header_t *)mbi_ptr;

    uint32_t current_adr = mbi_ptr + 8;

    while (current_adr < mbi_ptr + header->size) {
        tag_t *tag = (tag_t *)current_adr;
        if (tag->type == 0 && tag->size == 8)
            break;

        if (tag->type == 6) {
            for (int i = 0; i < 6; i++) {
                memory_map[i] = ((multiboot_tag_mmap *)tag)->entries[i];
            }
        }

        current_adr = current_adr + ((tag->size + 7) & ~7);
    }

    vmm_init();
    kprintf("Paging initialized.\n");
    init_idt();
    kprintf("IDT initialized.\n");
    init_pic();
    kprintf("PIC initialized.\n");
    __asm__ volatile("sti");
    init_pit(100);
    init_sched();
    init_tss();
}
