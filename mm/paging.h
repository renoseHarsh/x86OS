#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFF000

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define FOUR_MB 0x400000

#define P2V(addr) ((uint32_t)(addr) + KERNEL_VIRTUAL_BASE)
#define V2P(addr) ((uint32_t)(addr) - KERNEL_VIRTUAL_BASE)

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_SIZE_4MB 0x80

typedef uint32_t pde_t;
typedef uint32_t pte_t;

void init_paging();
