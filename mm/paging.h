#pragma once

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFF000
#define FOUR_MB 0x400000

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_SIZE_4MB 0x80
#define PAGE_WRITE_THROUGH 0x8
#define PAGE_NO_CACHE 0x10
#define PAGE_USER 0x4

typedef uint32_t pde_t;
typedef uint32_t pte_t;

extern pde_t page_directory[1024];

void refresh_cr3();
void map_range(uint32_t paddr, uint32_t vaddr, size_t size, uint32_t flag);
