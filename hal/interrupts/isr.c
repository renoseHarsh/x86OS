#include "debug.h"
#include "isr.h"
#include "kprintf.h"

#define IRQ_BASE 0x20 // Interrupt 32

static isr_t interrupt_handlers[256] = { 0 };

static const char *const g_Exceptions[] = { "Divide by zero error",
                                            "Debug",
                                            "Non-maskable Interrupt",
                                            "Breakpoint",
                                            "Overflow",
                                            "Bound Range Exceeded",
                                            "Invalid Opcode",
                                            "Device Not Available",
                                            "Double Fault",
                                            "Coprocessor Segment Overrun",
                                            "Invalid TSS",
                                            "Segment Not Present",
                                            "Stack-Segment Fault",
                                            "General Protection Fault",
                                            "Page Fault",
                                            "",
                                            "x87 Floating-Point Exception",
                                            "Alignment Check",
                                            "Machine Check",
                                            "SIMD Floating-Point Exception",
                                            "Virtualization Exception",
                                            "Control Protection Exception ",
                                            "",
                                            "",
                                            "",
                                            "",
                                            "",
                                            "",
                                            "Hypervisor Injection Exception",
                                            "VMM Communication Exception",
                                            "Security Exception",
                                            "" };

void __attribute__((cdecl)) interrupt_handler(register_t *regs)
{
    uint8_t vec = regs->interrupt;

    if (interrupt_handlers[vec]) {
        interrupt_handlers[vec](regs);
        return;
    }

    kprintf("\n\n");

    if (vec < 32)
        kprintf("Unhandled exception %d %s\n", vec, g_Exceptions[vec]);
    else
        kprintf("Unhandled interrupt: %d\n", vec);

    kprintf(
        "eax=%x ebx=%x ecx=%x edx=%x esi=%x edi=%x\n", regs->ax, regs->bx,
        regs->cx, regs->dx, regs->si, regs->di
    );
    kprintf("eip=%x cs=%x eflags=%x\n", regs->eip, regs->cs, regs->eflags);
    kprintf("interrupt=%d error_code=%x\n", vec, regs->error_code);
    kernel_panic();
}

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}
