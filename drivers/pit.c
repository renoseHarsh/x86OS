#include "pic.h"
#include "pit.h"
#include "ports.h"

static void pit_irq_handler(register_t *_)
{
    // Todo Scheduler
}

void init_pit(uint32_t hz)
{
    uint16_t divisor = 1193182 / hz;
    outb(0x43, 0x34); // channel 0, lobyte/hibyte access, rate generator
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    register_request_handler(0, pit_irq_handler);
}
