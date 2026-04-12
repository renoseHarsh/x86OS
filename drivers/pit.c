#include "pit.h"
#include "isr.h"
#include "pic.h"
#include "ports.h"
#include <stddef.h>
#include <stdint.h>

uint16_t divisor;
uint64_t ticks;

extern void scheduler();

void timer(register_t *_)
{
    ticks += divisor;
    scheduler();
}

void init_pit(uint32_t hz)
{
    divisor = PIT_CRYSTAL / hz;
    outb(0x43, 0x34); // channel 0, lobyte/hibyte access, rate generator
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    register_request_handler(0, &timer);
}
