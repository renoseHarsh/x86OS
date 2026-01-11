#include "paging.h"
#include "pmm.h"
#include <stdint.h>

pde_t *page_directory;
extern char _kernel_end[];

static void enable_paging()
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory));
    __asm__ volatile("mov %%cr0, %%eax\n\t"
                     "or $0x80000000, %%eax\n\t"
                     "mov %%eax, %%cr0" ::
                         : "eax", "memory");
}

static void identity_map_boot(uint32_t up_to_address)
{
    for (uint32_t addr = 0; addr < up_to_address; addr += PAGE_SIZE) {
        uint32_t pd_index = (addr >> 22) & 0x3FF;
        uint32_t pt_index = (addr >> 12) & 0x3FF;

        if (!(page_directory[pd_index] & PAGE_PRESENT)) {
            pte_t *new_page_table = (pte_t *)pmm_alloc_page();
            page_directory[pd_index]
                = ((uint32_t)new_page_table) | PAGE_PRESENT | PAGE_RW;
        }
        pte_t *page_table = (pte_t *)(page_directory[pd_index] & PAGE_MASK);
        page_table[pt_index] = (addr & PAGE_MASK) | PAGE_PRESENT | PAGE_RW;
    }
}

void init_paging()
{

    page_directory = (pde_t *)pmm_alloc_page();
    page_directory[1023] = ((uint32_t)page_directory) | PAGE_PRESENT | PAGE_RW;
    identity_map_boot((uint32_t)_kernel_end);
    enable_paging();
}
