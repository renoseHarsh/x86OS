#include "isr.h"
#include "panic.h"
#include "syscall.h"
#include "thread.h"

extern Thread *cur_thread;

void do_exit()
{
    cur_thread->status = TERMINATED;
    asm __volatile__("int $0x81");
    kernel_panic();
}

void syscall_dispatch(register_t *regs)
{
    switch (regs->eax) {
    case SYS_EXIT:
        do_exit();
        break;
    }
}

void init_syscalls()
{
    register_interrupt_handler(0x80, &syscall_dispatch);
}
