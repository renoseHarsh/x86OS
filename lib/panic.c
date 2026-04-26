#include "kprintf.h"
#include "panic.h"

void kernel_panic(char *err)
{
    __asm__ __volatile__("cli");
    kprintf("\n*** KERNEL PANIC ***\n");
    kprintf("%s\n", err);
    kprintf("System halted.\n");
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
