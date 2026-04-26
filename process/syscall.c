#include "isr.h"
#include "panic.h"
#include "syscall.h"
#include "thread.h"

extern Thread *cur_thread;
extern void (*on_thread_exit)(Thread *);

void do_exit()
{
    on_thread_exit(cur_thread);
    __asm__ volatile("int $0x81");
    kernel_panic("User process didn't exit");
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
