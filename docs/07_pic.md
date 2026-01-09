# Programmable interrupt controller (PIC)

PIC is a hardware device used in computers to manage and prioritize hardware interrupts. It allows multiple interrupt sources to be handled efficiently by the CPU. Outdated by APIC in modern systems.

## Sequence of operations
1. **Interrupt Request (IRQ)**: A hardware device sends an interrupt request signal by raising its **Interrupt Request** line (IR0-7) to the PIC.
2. **Interrupt Acknowledgment**: If the interrupt is not masked, the coresponding bit in Interrupt Request Register (IRR) is set. If a higher priority interrupt is not being serviced, the PIC sends an **Int** signal to the CPU.
3. **Acknowledge from CPU**: Once the CPU receives the interrupt signal, it checks the Interrupt Enable Flag (IF) in the EFLAGS register. If IF is set, the CPU sends an **Interrupt Acknowledge** signal back to the PIC, the pic then clears the corresponding bit in IRR and sets the corresponding bit in the In-Service Register (ISR).
4. **Second Interrupt Acknowledge**: The CPU sends a second **Int Acknowledge** signal, promting the PIC to send the interrupt vector to the CPU.
5. **End of Interrupt (EOI)**: After servicing the interrupt, the CPU sends an **End of Interrupt** command to the PIC, which clears the corresponding bit in the ISR.

Note: Since we are using Interrupt Gates in IDT, the cpu automatically clears the IF flag when servicing an interrupt, preventing further interrupts until the IRET instruction is executed. So that means we even when a lower priority interrupt is being serviced, the CPU doesn't send an Interrupt Acknowledge signal for a higher priority interrupt until the IRET instruction is executed.

## PIC Registers
- **Interrupt Request Register (IRR)**: Holds the pending interrupt requests.
- **In-Service Register (ISR)**: Holds the currently serviced interrupts.
- **Interrupt Mask Register (IMR)**: Used to mask (disable) specific interrupts.

## PIC I/O ports
- **Command Port**: 0x20 (Master), 0xA0 (Slave)
- **Data Port**: 0x21 (Master), 0xA1 (Slave)

## Ports to registers mapping
- Command Port -> IRR, ISR
- Data Port -> IMR

## Initialization
PICs need to be initialized using Initialization Command Words (ICWs) to set up their operation modes and vector offsets.
1. Send ICW1 to Command Port (0x20 for Master, 0xA0 for Slave)
2. Send ICW2 to Data Port (0x21 for Master, 0xA1 for Slave) to set vector offsets
3. Send ICW3 to Data Port to configure Master-Slave relationship
4. Send ICW4 to Data Port to set additional options

## Remapping PIC
By default, the PICs use interrupt vectors that conflict with CPU exceptions (0-31). To avoid this, we remap the PICs to use different interrupt vectors, typically starting from 32 (0x20) for the Master and 40 (0x28) for the Slave.

## Operation Command Words (OCWs)
- **OCW1**: Used to set the Interrupt Mask Register (IMR), sent to Data Port.
- **OCW2**: Used to send End of Interrupt (EOI) commands, sent to Command Port.
- **OCW3**: Used to read the IRR or ISR, sent to Command Port.

## Spurious Interrupts
Spurious interrupts can occur due to electrical noise or misconfigured hardware. The PIC sends a interrupt vector 0xFF for spurious interrupts, which should be handled appropriately in the interrupt handler. The CPU when it recieves interrupt 0 or 15 from the PIC, it should check the ISR to see if the interrupt is actually being serviced, if not it should ignore it, and not send an EOI.

## Warnings
- Make sure that vector offsets used in ICW2 are loaded into the IDT, and point to valid interrupt handlers, if it's not loaded it will cause a general protection fault when an interrupt occurs.

## References
- https://wiki.osdev.org/8259_PIC
- https://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf
- http://www.brokenthorn.com/Resources/OSDevPic.html
- https://helppc.netcore2k.net/hardware/8259 (Has some errors, be careful)