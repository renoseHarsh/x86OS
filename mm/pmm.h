#pragma once
#include "buddy/buddy.h"

#include <stddef.h>

static inline void *pmm_alloc(size_t order)
{
    return buddy_alloc_pages(order);
}

static inline void pmm_free(void *ptr)
{
    buddy_free_pages(ptr);
}
void pmm_init();
