#include "heap/heap.h"
#include "panic.h"
#include "thread.h"
#include <stdint.h>

extern pde_t kernel_page_directory[];
extern void (*on_thread_exit)(Thread *);

size_t thread_id = 1;

void kthread_stub(void (*entry_point)(void *), void *arg, Thread *thread)
{
    __asm__ volatile("sti");
    entry_point(arg);
    on_thread_exit(thread);
    __asm__ volatile("int $0x81");
    kernel_panic("Kernel Thread didn't exit");
}

Thread *kcreate_thread(void (*entry_point)(void *), void *arg)
{
    Thread *thread = kmalloc(sizeof(Thread));
    uint32_t *stackFrame = kmalloc(0x1000);
    uint32_t *sp = &stackFrame[1024];

    *--sp = (uint32_t)thread;
    *--sp = (uintptr_t)arg;
    *--sp = (uint32_t)entry_point;
    *--sp = 0x0; // fake return address for trampoline
    *--sp = (uint32_t)&kthread_stub;

    *--sp = 0; // eax
    *--sp = 0; // ecx
    *--sp = 0; // edx
    *--sp = 0; // ebx
    *--sp = 0; // esp
    *--sp = 0; // ebp
    *--sp = 0; // esi
    *--sp = 0; // edi

    thread->que.next = NULL;
    thread->que.prev = NULL;
    thread->id = thread_id++;
    thread->status = RUNNABLE;
    thread->kernel_stack_base = (uintptr_t)stackFrame;
    thread->kernel_esp = (uintptr_t)sp;
    thread->pd = kernel_page_directory;

    return thread;
}

void kdestroy_thread(Thread *thread)
{
    kfree((void *)thread->kernel_stack_base);
    kfree(thread);
}
