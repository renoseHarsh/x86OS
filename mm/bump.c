#include "bump.h"
#include "layout.h"
#include "string.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>

static uint32_t placement_addr;
void init_bump(uintptr_t end_addr)
{
    placement_addr = ALIGN_UP(end_addr, 0x1000);
}

void *bump_alloc(size_t num_pages)
{
    void *page = (void *)placement_addr;
    placement_addr += (0x1000 * num_pages);
    kmemset((void *)P2V(page), 0, 0x1000 * num_pages);
    return page;
}

uint32_t get_placement_addr()
{
    return placement_addr;
}
