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

memory_map_t *memory_map;

void perform_jump();

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
    perform_jump();
}

extern void jump_usermode(uintptr_t eip, uintptr_t esp);
void perform_jump()
{
    uintptr_t page = (uintptr_t)alloc_pages(0);
    map_range(page, 0x40000000, 1, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    refresh_cr3();
    char *code = (char *)0x40000000;
    code[0] = 0xCD; // int
    code[1] = 0x80; // 0x80
    code[2] = 0xEB; // jmp
    code[3] = 0xFE; // $
    uintptr_t user_stack = 0x40000000 + 0x1000;
    jump_usermode((uintptr_t)code, user_stack);
}
