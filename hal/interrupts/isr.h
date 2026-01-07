#pragma once

#include <stdint.h>

typedef struct {
    uint32_t di, si, bp, sp, bx, dx, cx, ax;
    uint32_t interrupt, error_code;
    uint32_t eip, cs, eflags;

} __attribute__((packed)) register_t;

void interrupt_handler(register_t *regs);
