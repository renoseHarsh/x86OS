#include "panic.h"
#include "ports.h"
#include "serial.h"

#define PORT 0x3F8

void seriel_init()
{
    outb(PORT + 1, 0x00); // Disable all interrupts
    outb(PORT + 3, 0x80); // Enable DLAB (set baud rate)
    outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00); //                   (hi byte)}
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if
                          // serial returns same byte)

    if (inb(PORT + 0) != 0xAE) {
        kernel_panic();
    }

    outb(PORT + 4, 0x0F);
}

int is_transmit_empty()
{
    return inb(PORT + 5) & 0x20;
}

void seriel_putc(char c)
{
    outb(PORT, c);
}
