# Multiboot2 & ELF Loading

To transition from a Ring 0-only environment to a system capable of running user-space applications, the kernel required a mechanism to load user binaries into memory.

## 1. The Bootloader Pivot (Why GRUB?)

The initial kernel iterations relied on a custom bootloader utilizing a basic ATA PIO driver to load the kernel binary at physical address `0x100000` (1 MiB). 

However, loading user-space programs requires parsing executables and setting up isolated memory spaces. Writing a custom bootloader capable of interacting with a file system to load multiple binaries is a complex undertaking that distracts from core kernel development. 

**The Solution:** The architecture was pivoted to become **Multiboot2 Compliant**. By adding a Multiboot2 header to the kernel assembly, the boot process is outsourced to GRUB. GRUB handles the hardware initialization, switches to 32-bit Protected Mode, and can load both the kernel and supplementary user programs (as "modules") directly into memory before the kernel even starts.

## 2. The Multiboot Information Structure (MBIS)

When GRUB hands control to the kernel, it leaves two critical pieces of data in the CPU registers:
*   `EAX`: The Multiboot2 Magic Number (`0x36D76289`).
*   `EBX`: A physical pointer to the Multiboot Information Structure (MBIS).

Because this is a Higher-Half kernel, the physical pointer in `EBX` cannot be dereferenced directly once paging is fully enabled. The kernel must add the higher-half virtual memory offset to this pointer to safely read the MBIS tags.

The kernel iterates through the MBIS tags to extract two vital pieces of information:
1.  **The Memory Map:** Replaces the custom E820 implementation to inform the Physical Memory Manager (Buddy Allocator) which memory frames are usable.
2.  **Loaded Modules:** Locates the physical addresses where GRUB loaded the user-space ELF binaries.

## 3. Parsing and In-Place Mapping the ELF Binary

When GRUB loads a user module, it places the raw ELF file into physical memory. Instead of wasting RAM and CPU cycles allocating new frames and copying this data, the kernel's ELF loader performs a highly efficient **zero-copy, in-place mapping**. 

### The Program Headers
The ELF header (`Elf32_Ehdr`) contains the offset (`e_phoff`) and count (`e_phnum`) of the **Program Headers**. The loader iterates over these headers to find segments marked as `PT_LOAD` (Loadable Segments).

For each `PT_LOAD` segment, the kernel performs the following:

1.  **Determine Permissions:** The `p_flags` field dictates the page table permissions.
    *   Executable (`PF_X`) & Read-Only (`PF_R`) ➔ Mapped as User Read-Only (`.text`, `.rodata`).
    *   Writable (`PF_W`) ➔ Mapped as User Read/Write (`.data`).
2.  **In-Place Mapping (Zero-Copy):** The loader calculates the physical address where GRUB placed this specific segment (`grub_module_start_phys + p_offset`). It then directly maps these existing physical frames into the new process's Page Directory at the virtual address specified by `p_vaddr`.
3.  **Handling the BSS (Uninitialized Data):** If `p_memsz` is strictly greater than `p_filesz`, the segment contains a `.bss` section. Because these variables take up no space in the ELF file, GRUB does not load them. The kernel must allocate *new* physical frames from the Buddy Allocator to cover the size difference, map them directly after the file-backed data, and explicitly zero them out using `memset`.
