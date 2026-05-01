#include "isr.h"
#include "pic.h"
#include "pit.h"
#include "ports.h"
#include <stddef.h>
#include <stdint.h>

uint64_t ticks;

timer_callback_t timer_callback;

void pit_irq_handler(register_t *_)
{
    pic_send_eoi(0);
    if (timer_callback)
        timer_callback();
}

uint16_t init_pit(uint32_t hz, timer_callback_t callback)
{
    uint16_t divisor = PIT_CRYSTAL / hz;
    outb(0x43, 0x34); // channel 0, lobyte/hibyte access, rate generator
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    timer_callback = callback;

    register_interrupt_handler(IRQ_BASE + 0, pit_irq_handler);
    return divisor;
}
