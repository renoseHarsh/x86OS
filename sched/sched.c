#include "dlist.h"
#include "gdt.h"
#include "heap/heap.h"
#include "isr.h"
#include "pit.h"
#include "process.h"
#include "sched.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

DList que;
DList sleepQue;
extern uint64_t ticks;
extern uintptr_t current_sp;
extern pde_t kernel_page_directory[];
extern TSS_ENTRY tss;
static Thread *idle_thread;
Thread *cur_thread = NULL;

void sched_enqueue(Thread *thread)
{
    list_push_back(&que, (Node *)thread);
}

Thread *get_next_thread()
{
    Thread *next = (Thread *)que.head;
    list_pop_front(&que);
    return next;
}

void wake_up_threads()
{
    while (sleepQue.head && ((Thread *)sleepQue.head)->wake_at <= ticks) {
        Node *cur = sleepQue.head;
        list_pop_front(&sleepQue);
        ((Thread *)cur)->status = RUNNABLE;
        list_push_back(&que, cur);
    }
}

void scheduler()
{
    cur_thread->kernel_esp = current_sp;
    pde_t *prev_pd = cur_thread->pd;
    if (cur_thread == idle_thread) {
    } else if (cur_thread->status == TERMINATED) {
        if (cur_thread->pd != kernel_page_directory) {
            refresh_cr3(kernel_page_directory);
            destroy_process(cur_thread);
        } else
            kdestroy_thread(cur_thread);
    } else if (cur_thread->status == SLEEPING) {
        sorted_list_push(&sleepQue, (Node *)cur_thread);
    } else {
        cur_thread->status = RUNNABLE;
        sched_enqueue(cur_thread);
    }
    wake_up_threads();
    Thread *next_thread = get_next_thread() ?: idle_thread;

    if (next_thread->pd != prev_pd) {
        refresh_cr3(next_thread->pd);
    }

    cur_thread = next_thread;
    tss.esp0 = cur_thread->kernel_stack_base + 0x1000;
    current_sp = cur_thread->kernel_esp;
}

void yield_interrupt_handler(register_t *_)
{
    scheduler();
}

void init_sched()
{
    cur_thread = kmalloc(sizeof(Thread));
    cur_thread->id = 0;
    cur_thread->que.next = NULL;
    cur_thread->que.prev = NULL;
    cur_thread->pd = kernel_page_directory;
    cur_thread->kernel_stack_base = 0;
    idle_thread = cur_thread;
    register_interrupt_handler(0x81, &yield_interrupt_handler);
}

void spawn(void (*entry_point)(void *), void *arg)
{
    Thread *thread = kcreate_thread(entry_point, arg);
    sched_enqueue(thread);
}

void sched_sleep(size_t time)
{
    uint64_t wake_at = ticks + ((uint64_t)time * PIT_CRYSTAL);
    cur_thread->wake_at = wake_at;
    cur_thread->status = SLEEPING;
    asm __volatile__("int $0x81");
}
