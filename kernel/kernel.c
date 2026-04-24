#include "bump.h"
#include "gdt.h"
#include "heap/heap.h"
#include "idt.h"
#include "kprintf.h"
#include "layout.h"
#include "paging.h"
#include "panic.h"
#include "pic.h"
#include "pit.h"
#include "pmm.h"
#include "process.h"
#include "sched.h"
#include "serial.h"
#include "vga.h"
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

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t start;
    uint32_t end;
    char cmdline[0];
} multiboot_tag_module;

memory_map_t *memory_map;
multiboot_tag_module *user_module;

void kmain(uint32_t magic, uint32_t mbi_ptr)
{
    seriel_init();
    vga_clear();
    if (magic != 0x36d76289) {
        kprintf("FATAL: Invalid Multiboot2 magic number: 0x%x\n", magic);
        kernel_panic();
    }

    header_t *header = (header_t *)P2V(mbi_ptr);

    uint32_t current_adr = (uintptr_t)header + 8;

    uint32_t placement_addr = (uint32_t)V2P(_kernel_start);

    if (mbi_ptr + header->size > placement_addr)
        placement_addr = mbi_ptr + header->size;

    while (current_adr < (uintptr_t)header + header->size) {
        tag_t *tag = (tag_t *)current_adr;
        if (tag->type == 0 && tag->size == 8)
            break;
        if (tag->type == 6)
            memory_map = ((multiboot_tag_mmap *)tag)->entries;
        else if (tag->type == 3) {
            user_module = (multiboot_tag_module *)tag;
            if (user_module->end > placement_addr)
                placement_addr = user_module->end;
        }

        current_adr = current_adr + ((tag->size + 7) & ~7);
    }

    init_bump(placement_addr);

    init_idt();
    kprintf("IDT initialized.\n");

    paging_init();
    kprintf("Paging initialized.\n");

    pmm_init();
    kprintf("PMM initialized\n");

    init_kmalloc();

    init_pic();
    kprintf("PIC initialized.\n");
    __asm__ volatile("sti");
    init_pit(100);
    init_sched();
    init_tss();
    create_process((void *)P2V(user_module->start));
}
