#include "sched.h"
#include "dlist.h"
#include "heap.h"
#include "isr.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

DList que;
extern uintptr_t current_sp;
Thread *cur_thread = NULL;

Thread *get_next_thread()
{
    Thread *next = (Thread *)que.head;
    list_pop_front(&que);
    return next;
}

void scheduler()
{
    if (!cur_thread) {
        cur_thread = kmalloc(sizeof(Thread));
        cur_thread->id = 0;
        cur_thread->que.next = NULL;
        cur_thread->que.prev = NULL;
    }
    if (cur_thread->status == TERMINATED) {
        free_stack(cur_thread);
        kfree(cur_thread);
    } else {
        cur_thread->esp = current_sp;
        cur_thread->status = RUNNABLE;
        sched_enqueue(cur_thread);
    }
    cur_thread = get_next_thread();
    cur_thread->status = RUNNING;
    current_sp = cur_thread->esp;
}

void yield_interrupt_handler(register_t *_)
{
    scheduler();
}

void init_sched()
{
    register_interrupt_handler(0x81, &yield_interrupt_handler);
}

void sched_enqueue(Thread *thread)
{
    list_push_back(&que, (Node *)thread);
}

void create_thread(void (*entry_point)(void *), void *arg)
{
    Thread *thread = kmalloc(sizeof(Thread));
    create_stack(entry_point, arg, thread);
    sched_enqueue(thread);
}
