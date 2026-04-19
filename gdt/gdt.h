#pragma once

#include <stdint.h>
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
} __attribute__((packed)) GDT_SEGMENT;

typedef struct {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) GDT_DESCRIPTOR;

typedef struct {
    uint32_t link;
    uint32_t esp0, ss0;
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt, iopb, ssp;
} __attribute__((packed)) TSS_ENTRY;

void init_gdt();
void init_tss();
