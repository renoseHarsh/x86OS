#pragma once

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define P2V(addr) ((uint32_t)(addr) + KERNEL_VIRTUAL_BASE)
#define V2P(addr) ((uint32_t)(addr) - KERNEL_VIRTUAL_BASE)

void *pmm_alloc_page();
