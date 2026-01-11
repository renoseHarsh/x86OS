#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFF000

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4

typedef uint32_t pde_t;
typedef uint32_t pte_t;

void init_paging();
