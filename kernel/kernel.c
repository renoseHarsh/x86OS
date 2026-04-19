#include "gdt.h"
#include "idt.h"
#include "kprintf.h"
#include "pic.h"
#include "pit.h"
#include "sched.h"
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

void test()
{
}
