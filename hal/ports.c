#include "ports.h"
#include <stdint.h>

void outb(uint16_t port, unsigned char val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "d"(port));
}

unsigned char inb(uint16_t port)
{
    unsigned char result;
    __asm__("inb %1, %0" : "=a"(result) : "d"(port));
    return result;
}
