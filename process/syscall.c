#include "isr.h"
#include "kprintf.h"
#include "reaper.h"
#include "sched.h"
#include "syscall.h"
#include "thread.h"
#include <stdint.h>

extern Thread *cur_thread;

static void do_exit()
{
    reaper_add(cur_thread);
    scheduler();
}

static void do_sleep(uint32_t time)
{
    thread_sleep(time);
}

static void syscall_handler(register_t *regs)
{
    switch (regs->eax) {
    case SYS_EXIT:
        do_exit();
        break;
    case SYS_SLEEP:
        do_sleep(regs->ebx);
        break;
    default:
        kprintf("unknown syscall\n");
        break;
    }
}

void init_syscall()
{
    register_interrupt_handler(0x80, syscall_handler);
}
