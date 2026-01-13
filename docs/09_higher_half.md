# Higher Half Kernel

Higher half kernel means mapping the kernel to the higher half of the virtual address space. In a 32-bit address space, this typically means mapping the kernel to start at the virtual address `0xC0000000` (3GB) and above, while user-space applications occupy the lower half (0x00000000 to 0xBFFFFFFF).

Now I am not sure if I would ever have user space applications, but this way I stay future proof and it also simplifies some aspects of memory management.

## Benefits of Higher Half Kernel
1. **Isolation**: By placing the kernel in the higher half, it is easier to isolate kernel memory from user-space memory. This helps prevent accidental or malicious access to kernel memory by user-space applications.
2. **Simplified Addressing**: The kernel can use fixed virtual addresses for its data structures and code, simplifying the addressing and reducing the need for complex address calculations.
3. **Easier Context Switching**: When switching between user mode and kernel mode, the CPU can maintain the same page tables, as the kernel is always mapped to the same higher half addresses.
4. **Compatibility with User Space**: If user-space applications are introduced later, the higher half kernel design will already be in place, making it easier to implement user-space memory management.

## Implementation
We can not just enable map the kernel to the higher half, and then enable paging, as our `eip` would still point to the lower half address space, while our code sits in the higher half.

To solve this we need a trampoline mechanism, that basically puts our code in the higher half, and then jumps to it.

1. **Prerequisites**: The following steps will make sense in the upcoming steps, the things we need are:
    - Update the linker script to place the kernel code and data sections at the higher half address space (e.g., starting at `0xC0000000`).
    - We must have jumped into the protected mode code.
    - The kernel jump must be the first steps in our kerenl, in this project it's handled in `kernel_entry.asm`.
    - We need to enable the `PSE` (Page Size Extension) bit in the `CR4` register to support 4MB pages.
2. **Page Directory Setup**: The first entry that covers the first 4MB of virtual memory is mapped to the first 4MB of physical memory. Allowing us to continue executing our code without issues. Fill up the direcotry with empty entries until we reach the entry that maps to `0xC0000000`, map that entry to the first 4MB of physical memory creating our higher half, and then continue filling up the rest of the directory with empty entries.
3. **Enable Paging**: Load the address of our page directory into the `CR3` register and set the paging bit in the `CR0` register to enable paging.
4. **Jump to Higher Half**: Now that paging is enabled, we can perform a far jump to the higher half address of our kernel entry point. This jump will switch the `EIP` to point to the higher half address space, allowing us to continue executing our kernel code in the higher half.
5. **Update Segment Selectors**: The current address for GDT in the `GDTR` register points to segment descriptors that use lower half addresses. We need to update the base address of our GDT to point to the virtual address of that GDT in the higher half. This ensures that when we switch to the higher half, the segment selectors still point to valid segments.
6. **Unmap Lower Half (Optional)**: If desired, we can unmap the lower half of the address space to prevent any accidental access. This step is optional and depends on the specific requirements of the operating system.

## Post Implementation
Now we need to create the final Page Directory and Page Tables for our kernel to use during normal operation. This involves setting up the necessary mappings for the kernel's code, data, stack, and any other required memory regions in the higher half address space.
1. **Things to consider**:
    - All our linker symbols for location points to higher half addresses, even `_kernel_end`.
    - We need macros to quickly convert between physical and virtual addresses.
2. **Create the Page Directory and Page Tables**:
    - Create a new Page Directory and Page Tables that mapa the lowerl physical kernel to the higher half virtual kernel.
    - The address in the entires has to physical addresses, so we need to use the macros to convert the higher half addresses to physical addresses.
    - Recursive mapping can be very useful here to easily access and modify page tables.
3. **Switch to the New Page Directory**: Load the address of the new Page Directory into the `CR3` register, make sure to put the physical address of the new Page Directory.

Note: The future allocator needs to handle physical to virtual mappings correctly, taking into account the higher half kernel mapping.

## References
- http://wiki.osdev.org/Higher_Half_x86_Bare_Bones
- http://wiki.osdev.org/Higher_Half_Kernel