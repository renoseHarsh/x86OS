#include "isr.h"
#include "pic.h"
#include "pit.h"
#include "ports.h"
#include <stddef.h>
#include <stdint.h>

uint16_t divisor;
uint64_t ticks;

extern void scheduler();

void timer(register_t *_)
{
    ticks += divisor;
    pic_send_eoi(0);
    scheduler();
}

void init_pit(uint32_t hz)
{
    divisor = PIT_CRYSTAL / hz;
    outb(0x43, 0x34); // channel 0, lobyte/hibyte access, rate generator
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    register_interrupt_handler(IRQ_BASE + 0, timer);
}
