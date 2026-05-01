#include "heap/heap.h"
#include "layout.h"
#include "paging.h"
#include "pmm.h"
#include "reaper.h"
#include "sched.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

size_t thread_id = 1;
extern pde_t kernel_page_directory[];

static void kthread_stub(void (*entry)(void *), void *arg, Thread *thread)
{
    __asm__ volatile("sti");
    entry(arg);
    reaper_add(thread);
    scheduler();
}

Thread *thread_create(void (*entry)(void *), void *arg)
{
    void *stack = pmm_alloc(0);
    Thread *thread = kmalloc(sizeof(Thread));

    uint32_t *esp = (uint32_t *)(P2V(stack) + 0x1000);

    *--esp = (uint32_t)thread;
    *--esp = (uint32_t)arg;
    *--esp = (uint32_t)entry;

    *--esp = 0x0;
    *--esp = (uint32_t)kthread_stub;

    *--esp = 0; // eax
    *--esp = 0; // ecx
    *--esp = 0; // edx
    *--esp = 0; // ebx
    *--esp = 0; // esp (ignored by popa)
    *--esp = 0; // ebp
    *--esp = 0; // esi
    *--esp = 0; // edi

    thread->esp = (uint32_t)esp;
    thread->stack = stack;
    thread->id = thread_id++;
    thread->pd = kernel_page_directory;

    thread_ready(thread);

    return thread;
}

void destroy_thread(Thread *thread)
{
    pmm_free(thread->stack);
    kfree(thread);
}
