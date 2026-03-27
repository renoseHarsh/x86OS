#include "idt.h"
#include "kprintf.h"
#include "pic.h"
#include "pit.h"
#include "serial.h"
#include "vga.h"
#include "vmm.h"
#include <stdbool.h>
#include <stdint.h>

void kmain()
{
    seriel_init();
    vga_clear();
    vmm_init();
    init_idt();
    kprintf("IDT initialized.\n");
    init_pic();
    kprintf("PIC initialized.\n");
    __asm__ volatile("sti");
    kprintf("Paging initialized.\n");
    init_pit(100);
}
