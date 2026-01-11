# Paging

Paging means mapping virtual addresses to physical addresses. This allows the OS to use memory more efficiently as it doesn't need to allocate contiguous physical memory for each process. It also provides isolation between processes, as each process can have its own virtual address space.

## Paging Structures
Paging in x86 architecture uses a multi-level page table structure. In 32-bit mode with 4KB pages, it typically uses a two-level page table structure consisting of:
1. **Page Directory**: Contains 1024 entries, each pointing to a Page Table.
2. **Page Table**: Each Page Table also contains 1024 entries, each pointing to a 4KB page in physical memory.

## Range coverage
Each entry in the Page Directory and Page Table covers a specific range of virtual addresses:
- **Page Table Entry (PTE)**: Each entry covers 4KB of virtual address space.
- **Page Directory Entry (PDE)**: Each entry covers 4MB of virtual address space (1024 Page Table entries * 4KB per page).
- **Entire Page Directory**: Covers 4GB of virtual address space (1024 Page Directory entries * 4MB per entry).

## Address Translation
To translate a virtual address to a physical address, the CPU performs the following steps:
1. **Extract the Page Directory Index (PDI)**: The highest 10 bits of the virtual address are used to index into the Page Directory.
2. **Extract the Page Table Index (PTI)**: The next 10 bits of the virtual address are used to index into the Page Table.
3. **Extract the Offset**: The lowest 12 bits of the virtual address are used as an offset within the 4KB page.
4. **Lookup in Page Directory** : The CPU uses the Page Directory Index to find the corresponding Page Directory Entry (PDE).
5. **Lookup in Page Table** : The CPU uses the Page Table Index to find the corresponding Page Table Entry (PTE).
6. **Calculate Physical Address** : The physical address is calculated by combining the frame address from the PTE with the offset.

## Page Table Entry (PTE) and Page Directory Entry (PDE) Format
Both PTEs and PDEs have a similar format, consisting of various flags and the frame address:
- **Present (P)**: Indicates if the page is present in physical memory.
- **Read/Write (R/W)**: Indicates if the page is writable.
- **User/Supervisor (U/S)**: Indicates if the page is accessible from user mode.
- **Page Frame Address**: The address of the physical page frame (the upper 20 bits in a 32-bit address).
- **Page Size (PS)**: In PDEs, indicates if the entry maps a 4MB page instead of a Page Table.

## Enabling Paging
To enable paging, the following steps are typically performed:
1. **Set up Page Tables**: Create and initialize the Page Directory and Page Tables in memory.
2. **Load Page Directory Base Register (CR3)**: Load the physical address of the Page Directory into the CR3 register.
3. **Enable Paging in CR0**: Set the PG (Paging) bit in the CR0 register to enable paging.
4. **Optional - Enable PSE**: If using 4MB pages, set the PSE (Page Size Extension) bit in CR4.

## TLB
The Translation Lookaside Buffer (TLB) is a cache used by the CPU to speed up virtual-to-physical address translation. It stores recent translations of virtual addresses to physical addresses, reducing the need to access the page tables in memory for every address translation.

## Note
- The Page Directory and all the Page Tables must be aligned to 4KB boundaries.
- If a page is not present (Present bit is 0), a page fault exception will be raised when accessing that page.
- TLB doesn't automatically update when page tables are modified. The OS must flush the TLB entries when necessary, using `invlpg` instruction or by reloading CR3.
- The kernel needs to be mapped before enabling paging to avoid page faults (usually done with identity mapping or higher-half kernel mapping).


## Recursive Page Table Mapping
To easily access page tables, a common technique is to use recursive page table mapping. This involves mapping the last entry of the Page Directory to point to the Page Directory itself. This way, the entire page table hierarchy can be accessed through the last 4MB of the virtual address space.
For example, if the last entry of the Page Directory (index 1023) points to the Page Directory itself, the virtual address range `0xFFC00000` to `0xFFFFFFFF` can be used to access all page tables and the page directory.
- Virtual address `0xFFFFF000` points to the Page Directory.
- Virtual address `0xFFC00000` points to the first Page Table.

## References
- http://wiki.osdev.org/Paging
- http://wiki.osdev.org/Setting_Up_Paging
- http://wiki.osdev.org/Page_Tables
- http://wiki.osdev.org/Memory_management
- http://wiki.osdev.org/Memory_Management_Unit