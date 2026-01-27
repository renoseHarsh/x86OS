#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "utils.h"

__attribute__((aligned(4096))) pde_t page_directory[1024];
extern char _kernel_end[];

void enable_paging()
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(V2P(page_directory)));
}

/*
Optimized mapping, when both the physical and virtual addresses are 4MiB
aligned, and the size is at least 4MiB, we map using 4MiB pages. Otherwise, we
fall back to 4KiB pages. Note: The 4kib transition relies on the fact that
after mapping a 4mib page, the pte is 0, so it can request the next page table
on the next iteration.
*/
void map_range(uint32_t paddr, uint32_t vaddr, size_t size, uint32_t flag)
{
    uint32_t pde = (vaddr >> 22) & 0x3FF;
    uint32_t pte = (vaddr >> 12) & 0x3FF;

    pte_t *page_table = NULL;

    while (size) {
        if (IS_ALIGNED(vaddr, FOUR_MB) && IS_ALIGNED(paddr, FOUR_MB)
            && size >= 1024) {
            page_directory[pde] = paddr | PAGE_SIZE_4MB | flag;
            paddr += FOUR_MB;
            vaddr += FOUR_MB;
            size -= 1024;
            pde++;
        } else {
            if (pte == 0 || !page_table) {
                if (!(page_directory[pde] & PAGE_PRESENT)) {
                    pde_t page_table_addr = (uint32_t)pmm_alloc_page();
                    page_directory[pde]
                        = page_table_addr | PAGE_PRESENT | PAGE_RW;
                }
                page_table = (pte_t *)P2V(page_directory[pde] & PAGE_MASK);
            }
            page_table[pte++] = paddr | flag;
            paddr += 0x1000;
            vaddr += 0x1000;
            pte = pte & (1024 - 1);
            if (!pte)
                pde++;
            size--;
        }
    }
}

void init_paging()
{
    kmemset(page_directory, 0, sizeof(page_directory));
    page_directory[1023] = V2P(page_directory) | PAGE_PRESENT | PAGE_RW;
}
