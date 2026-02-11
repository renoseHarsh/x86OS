#pragma once

#define MAX_BUDDY_ORDER 10

#define BASE_SHIFT 12
#define BASE_SIZE (1 << 12)

void init_buddy();
void *alloc_pages(const int order);
void free_pages(void *addr);
