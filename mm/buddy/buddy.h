#pragma once

#include "node.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Page {
    Node node;
    uint8_t order;
    uint8_t flags;
} Page;

#define P_FREE 0
#define P_TAIL (1 << 0)
#define P_USED (1 << 1)
#define P_KERNEL (1 << 2)

#define MAX_BUDDY_ORDER 10

#define BASE_SHIFT 12
#define BASE_SIZE (1 << 12)

void init_buddy();
void *alloc_pages(const int order);
void free_pages(void *addr);
size_t get_order_addr(uintptr_t addr);
