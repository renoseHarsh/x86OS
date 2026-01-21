#pragma once

#include "isr.h"
#include <stdint.h>
void init_pic();

void pic_send_eoi(uint8_t n);

void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

uint16_t pic_get_mask();

uint16_t pic_read_isr();
uint16_t pic_read_irr();

void register_request_handler(uint8_t n, isr_t handler);
