#include "idt.h"
#include "string.h"
#include <stdint.h>

__attribute__((aligned(0x10))) static idt_entry idt[256];
static idt_ptr idt_descriptor;
extern void *isr_stub_table[]; // Pointer to the array of ISR stubs in isr.asm

static void set_idt_entry(uint8_t vector, void *isr, uint8_t attributes)
{
    idt_entry *entry = &idt[vector];

    entry->isr_low = (uint16_t)((uint32_t)isr & 0xFFFF);
    entry->kernel_cs = 0x08; // Kernel code segment selector
    entry->zero = 0;
    entry->attributes = attributes;
    entry->isr_high = (uint16_t)((uint32_t)isr >> 16);
}

void init_idt()
{
    kmemset(&idt, 0, sizeof(idt_entry) * 256);
    for (int i = 0; i < 32; i++) {
        set_idt_entry(i, isr_stub_table[i], 0x8E);
    }

    idt_descriptor.limit = sizeof(idt_entry) * 256 - 1;
    idt_descriptor.base = (uint32_t)&idt;

    __asm__ volatile("lidtl (%0)" : : "r"(&idt_descriptor));
}
