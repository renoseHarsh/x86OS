#include "sched.h"
#include "dlist.h"
#include "heap.h"
#include "isr.h"
#include "pic.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

DList que;

Thread *get_next_thread()
{
    Thread *next = (Thread *)que.head;
    list_pop_front(&que);
    return next;
}

extern uintptr_t current_sp;
Thread *cur_thread = NULL;
void scheduler(register_t *_)
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

void init_sched()
{
    que.head = NULL;
    que.tail = NULL;
    que.size = 0;
    register_request_handler(0, &scheduler);
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
