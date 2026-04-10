#include "pit.h"
#include "ports.h"
#include <stddef.h>
#include <stdint.h>

void init_pit(uint32_t hz)
{
    uint16_t divisor = 1193182 / hz;
    outb(0x43, 0x34); // channel 0, lobyte/hibyte access, rate generator
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}
