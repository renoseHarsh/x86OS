#include "bump.h"
#include "layout.h"
#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>

extern memory_map_t *memory_map;
__attribute__((aligned(4096))) pde_t kernel_page_directory[1024];

void refresh_cr3(pde_t pd[])
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(V2P(pd)));
}

void early_map_range(
    pde_t pd[], uint32_t paddr, uint32_t vaddr, size_t size, uint32_t flag
)
{
    uint32_t pde = (vaddr >> 22) & 0x3FF;
    uint32_t pte = (vaddr >> 12) & 0x3FF;

    pte_t *page_table = NULL;

    while (size) {
        if (IS_ALIGNED(vaddr, FOUR_MB) && IS_ALIGNED(paddr, FOUR_MB)
            && size >= 1024) {
            pd[pde] = paddr | PAGE_SIZE_4MB | flag;
            paddr += FOUR_MB;
            vaddr += FOUR_MB;
            size -= 1024;
            pde++;
        } else {
            if (pte == 0 || !page_table) {
                if (!(pd[pde] & PAGE_PRESENT)) {
                    pde_t page_table_addr = (uint32_t)bump_alloc(1);
                    pd[pde] = page_table_addr | PAGE_PRESENT | PAGE_RW | flag;
                }
                page_table = (pte_t *)P2V(pd[pde] & PAGE_MASK);
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

static void map_bios()
{

    // 1. IVT, BDA (0x0 -> 0x1000)
    // Read-Only to protect NULL pointer and BIOS data
    early_map_range(kernel_page_directory, 0x0, P2V(0x0), 1, PAGE_PRESENT);

    // 2. Free Usable Memory (0x1000 -> 0x80000)
    // 127 Pages (508 KiB)
    early_map_range(
        kernel_page_directory, 0x1000, P2V(0x1000),
        (0x80000 - 0x1000) / 0x1000, PAGE_PRESENT | PAGE_RW
    );

    // 3. EBDA Safe Zone (0x80000 -> 0xA0000)
    // 32 Pages (128 KiB) - Read Only
    early_map_range(
        kernel_page_directory, 0x80000, P2V(0x80000),
        (0xA0000 - 0x80000) / 0x1000, PAGE_PRESENT
    );

    // 4. VGA Video Memory (0xA0000 -> 0xC0000)
    // 32 Pages (128 KiB) - RW + Cache Disable
    early_map_range(
        kernel_page_directory, 0xA0000, P2V(0xA0000),
        (0xC0000 - 0xA0000) / 0x1000,
        PAGE_PRESENT | PAGE_RW | PAGE_NO_CACHE | PAGE_WRITE_THROUGH
    );

    // 5. Video BIOS, Expansions, Motherboard BIOS (0xC0000 -> 0x100000)
    // 64 Pages (256 KiB) - Read Only + Cache Disable
    early_map_range(
        kernel_page_directory, 0xC0000, P2V(0xC0000),
        (0x100000 - 0xC0000) / 0x1000, PAGE_PRESENT | PAGE_NO_CACHE
    );
}

static void map_kernel()
{
    uint32_t ro_start = V2P(_ro_start);
    uint32_t ro_end = V2P(_ro_end);
    uint32_t rw_start = V2P(_rw_start);
    uint32_t rw_end = V2P(_rw_end);
    uint32_t kernel_end = V2P(_kernel_end);

    // Map the code  and read only data segments
    early_map_range(
        kernel_page_directory, ro_start, P2V(ro_start),
        (ro_end - ro_start) / 0x1000, PAGE_PRESENT
    );

    // Map the .bss and read-write data segments
    early_map_range(
        kernel_page_directory, rw_start, P2V(rw_start),
        (rw_end - rw_start) / 0x1000, PAGE_PRESENT | PAGE_RW
    );

    // Map the remaining free memory in RAM after the kernel
    uint32_t ram_phys_end = 0x100000 + memory_map[3].size;
    uint32_t free_size = ram_phys_end - kernel_end;
    early_map_range(
        kernel_page_directory, kernel_end, P2V(kernel_end), free_size / 0x1000,
        PAGE_PRESENT | PAGE_RW
    );

    // Map the ACPI Reclaimable Memory
    early_map_range(
        kernel_page_directory, memory_map[4].addr, P2V(memory_map[4].addr),
        memory_map[4].size / 0x1000, PAGE_PRESENT
    );
}

void paging_init()
{

    kmemset(kernel_page_directory, 0, sizeof(kernel_page_directory));
    map_bios();
    map_kernel();
    refresh_cr3(kernel_page_directory);
}

void map_range(
    pde_t pd[], uint32_t paddr, uint32_t vaddr, size_t size, uint32_t flag
)
{
    uint32_t pde = (vaddr >> 22) & 0x3FF;
    uint32_t pte = (vaddr >> 12) & 0x3FF;

    pte_t *page_table = NULL;

    while (size) {
        if (IS_ALIGNED(vaddr, FOUR_MB) && IS_ALIGNED(paddr, FOUR_MB)
            && size >= 1024) {
            pd[pde] = paddr | PAGE_SIZE_4MB | flag;
            paddr += FOUR_MB;
            vaddr += FOUR_MB;
            size -= 1024;
            pde++;
        } else {
            if (pte == 0 || !page_table) {
                if (!(pd[pde] & PAGE_PRESENT)) {
                    pde_t page_table_addr = (uint32_t)pmm_alloc(0);
                    pd[pde] = page_table_addr | PAGE_PRESENT | PAGE_RW | flag;
                }
                page_table = (pte_t *)P2V(pd[pde] & PAGE_MASK);
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
