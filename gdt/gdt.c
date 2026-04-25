#include "gdt.h"
#include <stddef.h>
#include <stdint.h>

extern void gdt_flush(uint32_t gdt_ptr);

GDT_SEGMENT gdt[6];
GDT_DESCRIPTOR gdtr;
TSS_ENTRY tss;

uint8_t kernel_stack[0x1000];

void fill_gdt(
    size_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags
)
{
    gdt[idx].base_low = (uint16_t)base;
    gdt[idx].base_mid = (uint8_t)(base >> 16);
    gdt[idx].base_high = (uint8_t)(base >> 24);

    gdt[idx].limit_low = (uint16_t)limit;
    gdt[idx].limit_high_flags = ((0xF & flags) << 4) | (0xF & (limit >> 16));

    gdt[idx].access = access;
}

void init_gdt()
{
    fill_gdt(0, 0, 0, 0, 0); // Null Descriptor

    // kernel segments
    fill_gdt(1, 0, 0xFFFFF, 0b10011010, 0b1100); // Code Segment
    fill_gdt(2, 0, 0xFFFFF, 0b10010010, 0b1100); // Data Segment

    fill_gdt(3, 0, 0xFFFFF, 0b11111010, 0b1100); // Code Segment
    fill_gdt(4, 0, 0xFFFFF, 0b11110010, 0b1100); // Data Segment

    fill_gdt(5, (uint32_t)&tss, sizeof(tss) - 1, 0b10001001, 0b0000); // TSS

    gdtr.offset = (uint32_t)gdt;
    gdtr.size = sizeof(gdt) - 1;

    gdt_flush((uint32_t)&gdtr);
}

void init_tss()
{
    tss.ss0 = (2 << 3) | 0 | 0;
    __asm__ volatile("ltr %0" ::"r"((uint16_t)((uintptr_t)&gdt[5]
                                               - (uintptr_t)&gdt[0])));
}
