#include "idt.h"
#include "kprintf.h"
#include "paging.h"
#include "pic.h"
#include "vga.h"
#include <stdbool.h>
#include <stdint.h>

int kmain()
{
    vga_clear();
    init_idt();
    kprintf("IDT initialized.\n");
    init_pic();
    kprintf("PIC initialized.\n");
    __asm__ volatile("sti");
    init_paging();
    kprintf("Paging initialized.\n");
    for (;;) {
    }
}
