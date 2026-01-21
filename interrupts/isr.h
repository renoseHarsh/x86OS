#pragma once

#include <stdint.h>

typedef struct {
    uint32_t di, si, bp, sp, bx, dx, cx, ax;
    uint32_t interrupt, error_code;
    uint32_t eip, cs, eflags;

} __attribute__((packed)) register_t;

void interrupt_handler(register_t *regs);
typedef void (*isr_t)(register_t *); // Fucntion pointer to ISR handler
void register_interrupt_handler(uint8_t n, isr_t handler);
