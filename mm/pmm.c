#include "paging.h"
#include "pmm.h"
#include "string.h"
#include <stdint.h>

extern char _kernel_end[];

// No need to align, as _kernel_end is already page-aligned
static uint32_t placement_addr = (uint32_t)V2P(_kernel_end);

void *pmm_alloc_page()
{
    void *page = (void *)placement_addr;
    placement_addr += 0x1000; // Advance by one page (4KB)
    kmemset((void *)P2V(page), 0, 0x1000);
    return page;
}
