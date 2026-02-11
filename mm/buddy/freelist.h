#pragma once

#include <stdint.h>

typedef struct Page {
    struct Page *next;
    struct Page *prev;
    uint8_t order;
    uint8_t flags;
} Page;

#define P_FREE 0
#define P_TAIL (1 << 0)
#define P_USED (1 << 1)
#define P_KERNEL (1 << 2)

void push_node(Page **head_ref, Page *node);
void remove_node(Page **head_ref, Page *node);
Page *pop_node(Page **head_ref);
