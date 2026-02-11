#include "idt.h"
#include "kprintf.h"
#include "pic.h"
#include "vga.h"
#include "vmm.h"
#include <stdbool.h>
#include <stdint.h>

void kmain()
{
    vga_clear();
    init_idt();
    kprintf("IDT initialized.\n");
    init_pic();
    kprintf("PIC initialized.\n");
    __asm__ volatile("sti");
    vmm_init();
    kprintf("Paging initialized.\n");
}
