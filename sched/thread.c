#include "heap/heap.h"
#include "panic.h"
#include "thread.h"
#include <stdint.h>

size_t thread_id = 1;

void thread_stub(void (*entry_point)(void *), void *arg, Thread *thread)
{
    entry_point(arg);
    thread->status = TERMINATED;
    asm __volatile__("int $0x81");
    kernel_panic();
}

void create_stack(void (*entry_point)(void *), void *arg, Thread *thread)
{
    uint32_t *stackFrame = kmalloc(0x1000);
    uint32_t *sp = &stackFrame[1024];

    *--sp = (uint32_t)thread;
    *--sp = (uintptr_t)arg;
    *--sp = (uint32_t)entry_point;
    *--sp = 0x0; // fake return address for trampoline

    uint32_t threadesp = (uint32_t)sp;

    *--sp = (1 << 9);               // eflags interrupt enabled
    *--sp = 0x08;                   // CS
    *--sp = (uint32_t)&thread_stub; // eip

    *--sp = 0; // error_code
    *--sp = 0; // interrupt

    for (int i = 0; i < 4; i++) {
        *--sp = 0;
    } // eax, ecx, edx, ebx

    *--sp = threadesp; // esp

    *--sp = (uint32_t)&stackFrame[1024]; // ebp
    *--sp = 0;                           // esi
    *--sp = 0;                           // edi

    thread->status = RUNNABLE;
    thread->id = thread_id++;
    thread->que.next = NULL;
    thread->que.prev = NULL;
    thread->stack = (uintptr_t)stackFrame;
    thread->esp = (uintptr_t)sp;
}

void free_stack(Thread *thread)
{
    kfree((void *)thread->stack);
}
