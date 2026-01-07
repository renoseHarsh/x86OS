#pragma once

#include <stdint.h>

void outb(uint16_t port, unsigned char val);
unsigned char inb(uint16_t port);
