#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "vmm.h"

void vmm_init()
{
    kmemset(page_directory, 0, sizeof(page_directory));
    page_directory[1023] = V2P(page_directory) | PAGE_PRESENT | PAGE_RW;
    map_range(
        0, KERNEL_VIRTUAL_BASE, 0x40000 - 0x1000, PAGE_PRESENT | PAGE_RW
    );
    refresh_cr3();
}
