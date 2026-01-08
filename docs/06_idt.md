# Interrupt Descriptor Table (IDT)

It's basically a list of functions that cpu can call when certain events happen, like hardware interrupts or exceptions. Each entry in the IDT is called a descriptor and contains information about how to handle a specific interrupt or exception.

## IDT Register
The location IDT is kept in the IDT register (IDTR). The IDTR holds the base address and limit of the IDT, allowing the CPU to locate and access the table when needed. To load the IDTR with the address of the IDT, we use the `lidt` instruction.

## IDT Entry Structure

1. **Offset**: The address of the interrupt handler function.
2. **Selector**: A code segment selector where the handler function resided (the cpu switched to this selector before calling the function).
3. **Type and Attributes**: Information about the type of interrupt (e.g., interrupt gate, trap gate) and its attributes (e.g., privilege level).
4. **Zero**: Reserved field, typically set to zero.

**Gate Type**: There are two main types of gates used in the IDT, interrupt gates and trap gates. The key difference is that interrupt gates disable further interrupts when the handler is called, while trap gates do not. interrupt gates are typically used for hardware interrupts, while trap gates are used for exceptions and software interrupts. (Note: We will exclusively use interrupt gates for both hardware interrupts and exceptions for now, since we want the kernel to panic and halt at excpetion not, service other interrupts.)

## Setting Up the IDT
To set up the IDT, we need to:
1. Define the IDT structure and its entries.
2. Create interrupt handler functions for various interrupts and exceptions.
3. Populate the IDT with the appropriate entries, linking each interrupt or exception to its handler
4. Load the IDT using the `lidt` instruction.

## My approach
In my implementation, I have `isr_stub.asm` this creates a macro for wrapper function which we duplicated 256 times (for 0-255 interrupts). There two distinct wrapper function, one which pushes an dummy error code for interrupts that do not push an error code, and one which does not push a dummy error code for interrupts that already push an error code. This keeps arguments consistent for the common handler function `interrupt_handler` in `isr.c`.

The arguments pushed to `interrupt_handler` are of two types the first kind are pushed by the wrapper function and the other kind are pushed by the cpu automatically when an interrupt occurs.

- **CPU pushed arguments:** eflags->CS->EIP->Error Code (if applicable)
- **Wrapper pushed arguments:** dummy error code (if applicable) -> interrupt number -> pusha'ed registers, and the pointer to the stack frame, so the `interrupt_handler` can access all the pushed values.

Note: If we have a different DS in future than the kernel DS, we need to push the correct DS value in the wrapper function before pushing other registers.

### `interrupt_handler`

Common function to handle all interrupts and exceptions.

**Inputs**
- `registers_t *regs`: A pointer to a structure containing all the cpu pushed common stub pushed information.

**Behavior**
1. Use interrupt number from `regs` to index into `interrupt_handlers` array.
2. If a handler is registered for that interrupt number, call it with `regs` as an argument.
3. If no handler is registered, perform default action (log the event and panic)

**Dispatch table**
- The `interrupt_handlers` array is a dispatch table where each index corresponds to an interrupt number.
- Defined in `isr.c`
- Has a register api `register_interrupt_handler` to allow other parts of the kernel to register their own handlers for specific interrupts.
- Notes:
    - All interrupt paths come through this function.