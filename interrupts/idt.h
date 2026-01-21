#include <stdint.h>

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr;

typedef struct {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t zero; // Reserved, set to 0
    uint8_t attributes;
    uint16_t isr_high;
} __attribute__((packed)) idt_entry;

void init_idt();
