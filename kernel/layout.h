#pragma once

#include <stdint.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define P2V(addr) ((uint32_t)(addr) + KERNEL_VIRTUAL_BASE)
#define V2P(addr) ((uint32_t)(addr) - KERNEL_VIRTUAL_BASE)

extern char _kernel_start[];
extern char _kernel_end[];
extern char _ro_start[];
extern char _ro_end[];
extern char _rw_start[];
extern char _rw_end[];

typedef struct {
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed)) memory_map_t;
