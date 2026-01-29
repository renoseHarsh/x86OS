#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "vmm.h"
#include <stdint.h>

static memory_map_t *memory_map = (memory_map_t *)(P2V(0x8004));

static void map_bios()
{
    // 1. IVT, BDA (0x0 -> 0x1000)
    // Read-Only to protect NULL pointer and BIOS data
    map_range(0x0, P2V(0x0), 1, PAGE_PRESENT);

    // 2. Free Usable Memory (0x1000 -> 0x80000)
    // 127 Pages (508 KiB)
    map_range(
        0x1000, P2V(0x1000), (0x80000 - 0x1000) / 0x1000,
        PAGE_PRESENT | PAGE_RW
    );

    // 3. EBDA Safe Zone (0x80000 -> 0xA0000)
    // 32 Pages (128 KiB) - Read Only
    map_range(
        0x80000, P2V(0x80000), (0xA0000 - 0x80000) / 0x1000, PAGE_PRESENT
    );

    // 4. VGA Video Memory (0xA0000 -> 0xC0000)
    // 32 Pages (128 KiB) - RW + Cache Disable
    map_range(
        0xA0000, P2V(0xA0000), (0xC0000 - 0xA0000) / 0x1000,
        PAGE_PRESENT | PAGE_RW | PAGE_NO_CACHE | PAGE_WRITE_THROUGH
    );

    // 5. Video BIOS, Expansions, Motherboard BIOS (0xC0000 -> 0x100000)
    // 64 Pages (256 KiB) - Read Only + Cache Disable
    map_range(
        0xC0000, P2V(0xC0000), (0x100000 - 0xC0000) / 0x1000,
        PAGE_PRESENT | PAGE_NO_CACHE
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
    map_range(
        ro_start, P2V(ro_start), (ro_end - ro_start) / 0x1000, PAGE_PRESENT
    );

    // Map the .bss and read-write data segments
    map_range(
        rw_start, P2V(rw_start), (rw_end - rw_start) / 0x1000,
        PAGE_PRESENT | PAGE_RW
    );

    // Map the remaining free memory in RAM after the kernel
    uint32_t ram_phys_end = 0x100000 + memory_map[3].size;
    uint32_t free_size = ram_phys_end - kernel_end;
    map_range(
        kernel_end, P2V(kernel_end), free_size / 0x1000, PAGE_PRESENT | PAGE_RW
    );

    // Map the ACPI Reclaimable Memory
    map_range(
        memory_map[4].addr, P2V(memory_map[4].addr),
        memory_map[4].size / 0x1000, PAGE_PRESENT
    );
}

void vmm_init()
{
    kmemset(page_directory, 0, sizeof(page_directory));
    page_directory[1023] = V2P(page_directory) | PAGE_PRESENT | PAGE_RW;
    map_range(
        0, KERNEL_VIRTUAL_BASE, 0x40000 - 0x1000, PAGE_PRESENT | PAGE_RW
    );
    refresh_cr3();
}
