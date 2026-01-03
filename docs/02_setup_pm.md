# Protected Mode Setup

This is the powerful mode of the x86 architecture that allows the OS to use features like virtual memory, paging, and access control.

Steps to switch to protected mode:
1. **Disable Interrupts**: Before switching modes, it's essential to disable interrupts to prevent any unexpected behavior during the  transition.
2. **A20-bit Enable**: The A20 line must be enabled to access memory above 1MB. This is typically done by interacting with the keyboard controller or using BIOS services.
3. **Load GDT (Global Descriptor Table)**: The GDT defines the characteristics of the various memory segments (code, data, stack) used in protected mode. We need to create a GDT and load it using the `lgdt` instruction.
4. **Set the PE (Protection Enable) Bit**: This is done by modifying the CR0 control register to set the PE bit, which enables protected mode.
5. **Far Jump to Protected Mode**: After setting the PE bit, a far jump is performed to flush the instruction pipeline and start executing in protected mode, which also sets CS to the code segment defined in the GDT.
6. **Set Up Segment Registers**: After the jump, the segment registers (DS, ES, FS, GS, SS) need to be initialized to point to the appropriate segments defined in the GDT.

- Steps 1 to 3 are implemented in `boot/pm_setup.asm`.
- Steps 4 to 5 are implemented in `boot/boot.asm` after calling the setup function from `boot/pm_setup.asm`.
- Step 6 is implemented in `boot/stage2.asm` after the far jump to protected mode.

`boot/stage2.asm` is the second stage bootloader that continues the boot process in protected mode.

## Important Notes:
- The gdt will have two segment descriptors, code and data segment, both covering the entire 4GB address space with appropriate access rights. It's minimal since we willl be using paging later for memory management.

- Any misconfiguration in the GDT or incorrect setup steps can lead to system crashes or unexpected behavior, so it's crucial to follow each step carefully, like triple fault which will reset the CPU.

Reference:
- https://wiki.osdev.org/Protected_Mode
- https://wiki.osdev.org/Global_Descriptor_Table
- https://wiki.osdev.org/A20_Line

