#include "idt.h"
#include "kprintf.h"
#include "vga.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
int kmain()
{
    vga_clear();
    init_idt();
    kprintf("IDT initialized.\n");

    for (;;) {
    }
}
