#include "kprintf.h"
#include "vga.h"
#include <stdbool.h>
#include <stdint.h>
int kmain()
{
    vga_clear();
    kprintf("Hello, Kernel World!\n");

    for (;;) {
    }
}
