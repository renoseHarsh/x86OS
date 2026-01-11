#include "idt.h"
#include "isr.h"
#include "kprintf.h"
#include "paging.h"
#include "pic.h"
#include "vga.h"
#include <stdbool.h>
#include <stdint.h>

void timer(register_t *regs)
{
    (void)regs;
}

int kmain()
{
    vga_clear();
    init_idt();
    kprintf("IDT initialized.\n");
    init_pic();
    kprintf("PIC initialized.\n");
    register_interrupt_handler(32, timer);
    __asm__ volatile("sti");
    init_paging();
    kprintf("Paging initialized.\n");
    for (;;) {
    }
}
