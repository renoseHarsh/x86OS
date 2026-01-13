#include "paging.h"
#include "pmm.h"
#include <stdint.h>

pde_t *page_directory;
extern char _kernel_end[];

static void enable_paging()
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(V2P(page_directory)));
}

static void map_kernel()
{
    uint32_t real_start = 0;
    uint32_t virtual_start = KERNEL_VIRTUAL_BASE;

    uint32_t virtual_end = (uint32_t)_kernel_end;

    for (uint32_t vaddr = virtual_start, paddr = real_start;
         vaddr < virtual_end;) {

        uint16_t pde_i = (vaddr >> 22) & 0X3FF;
        uint16_t pte_i = (vaddr >> 12) & 0x3FF;

        if (!(page_directory[pde_i] & PAGE_PRESENT)) {
            pde_t page_table = (uint32_t)pmm_alloc_page();
            page_directory[pde_i] = (page_table | (PAGE_PRESENT | PAGE_RW));
        }
        pte_t *page_table = (pte_t *)P2V((page_directory[pde_i] & PAGE_MASK));
        page_table[pte_i] = paddr | PAGE_PRESENT | PAGE_RW;

        vaddr += 0x1000;
        paddr += 0x1000;
    }
}

void init_paging()
{

    uint32_t pd_phys_addr = (uint32_t)pmm_alloc_page();
    page_directory = (pde_t *)P2V(pd_phys_addr);
    page_directory[1023] = pd_phys_addr | PAGE_PRESENT | PAGE_RW;
    map_kernel();
    enable_paging();
}
