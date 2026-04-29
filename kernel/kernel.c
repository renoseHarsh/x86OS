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
#include "reaper.h"
#include "sched.h"
#include "serial.h"
#include "thread.h"
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

void somebody()
{
}

void real_test()
{
    kprintf("creating %u\n", spawn((void *)somebody, NULL)->id);
    thread_sleep(5);
    kprintf("\n\n");

    kprintf("creating %u\n", spawn((void *)somebody, NULL)->id);
    thread_sleep(5);
    kprintf("\n\n");

    kprintf("creating %u\n", spawn((void *)somebody, NULL)->id);
    thread_sleep(5);
    kprintf("\n\n");
}

void print_digit(size_t num, size_t row, size_t *col)
{
    if (num >= 10)
        print_digit(num / 10, row, col);
    char c = (num % 10) + '0';
    vga_putc_at(c, row, *col);
    (*col)++;
}

void print_num(void *arg)
{
    size_t row = (size_t)arg;
    for (uint32_t num = UINT32_MAX - 10000; num < UINT32_MAX; num++) {
        // size_t num = 0;
        // while (true) {
        size_t col = 0;
        print_digit(num, row, &col);
    }
}

memory_map_t *memory_map;
multiboot_tag_module *user_module;
void kmain(uint32_t magic, uint32_t mbi_ptr)
{
    seriel_init();
    vga_clear();
    if (magic != 0x36d76289) {
        kernel_panic("Invalid Multiboot2 Magic number");
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
    init_pit(100);
    init_tss();

    Thread *main_thread = init_sched();
    init_reaper();
    // for (int i = 0; i < 8; i++) {
    //     spawn(print_num, (void *)17 + i);
    // }
    spawn((void *)real_test, NULL);
    main_thread->status = IDLE;
    __asm__ volatile("sti");
}
