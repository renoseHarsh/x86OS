#include "pic.h"
#include "ports.h"
#include <stdint.h>

#define PIC1 0x20 // IO base address for master PIC
#define PIC2 0xA0 // IO base address for slave PIC

#define IRQ_BASE 0x20 // Interrupt 32

#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1 0x11 // Has ICW4, edge triggered, cascade mode

// Base of vector in IDT to use
#define ICW2_MASTER IRQ_BASE    // Interrupt 32-39
#define ICW2_SLAVE IRQ_BASE + 8 // Interrupt 40-47

#define ICW3_MASTER 0x04 // Attach slave to IRQ 2 on master
#define ICW3_SLAVE 0x02  // Slave is attached on IRQ 2 on master

#define ICW4 0x01 // Set to x86 mode

#define OCW3_IRR 0x0A
#define OCW3_ISR 0x0B

static uint16_t pic_mask = 0xFFFF;

void init_pic()
{
    // Send ICW1 to both command registers
    outb(PIC1_COMMAND, ICW1);
    outb(PIC2_COMMAND, ICW1);
    io_wait();

    // Send ICW2 to both data registers
    outb(PIC1_DATA, ICW2_MASTER);
    outb(PIC2_DATA, ICW2_SLAVE);
    io_wait();

    // Send ICW3 to both data registers
    outb(PIC1_DATA, ICW3_MASTER);
    outb(PIC2_DATA, ICW3_SLAVE);
    io_wait();

    // Send ICW4 to both data registers
    outb(PIC1_DATA, ICW4);
    outb(PIC2_DATA, ICW4);
    io_wait();

    // Read the mask
    pic_mask = (inb(PIC2_DATA) << 8) | inb(PIC1_DATA);
}

void pic_send_eoi(uint8_t n)
{
    // Special handling for spurious IRQs
    if (n == 7 || n == 15) {
        if ((pic_read_isr() & (1u << n)) == 0) {
            if (n >= 8) {
                outb(PIC1_COMMAND, 0x20);
            }
            return;
        }
    }

    // Send the EOI command (0x20) to command registers
    if (n >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}

static inline void pic_write_mask()
{
    outb(PIC1_DATA, (uint8_t)pic_mask & 0xFF);
    outb(PIC2_DATA, (uint8_t)pic_mask >> 8);
}

uint16_t pic_get_mask()
{
    return pic_mask;
}

void pic_mask_irq(uint8_t irq)
{
    pic_mask |= (1u << irq);
    pic_write_mask();
}

void pic_unmmask_rqp(uint8_t irq)
{
    pic_mask &= ~(1u << irq);

    if ((pic_mask & 0xFF00) != 0xFF00)
        pic_mask &= ~(1u << 2);

    pic_write_mask();
}

uint16_t pic_read_isr()
{
    outb(PIC1_COMMAND, OCW3_ISR);
    outb(PIC2_COMMAND, OCW3_ISR);
    io_wait();
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t pic_read_irr()
{
    outb(PIC1_COMMAND, OCW3_IRR);
    outb(PIC2_COMMAND, OCW3_IRR);
    io_wait();
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}
