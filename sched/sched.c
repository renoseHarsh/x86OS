#include "sched.h"
#include "dlist.h"
#include "heap.h"
#include "isr.h"
#include "pit.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

DList que;
DList sleepQue;
extern uint64_t ticks;
extern uintptr_t current_sp;
Thread *cur_thread = NULL;

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
    if (cur_thread->status == TERMINATED) {
        free_stack(cur_thread);
        kfree(cur_thread);
    } else if (cur_thread->status == SLEEPING) {
        cur_thread->esp = current_sp;
        sorted_list_push(&sleepQue, (Node *)cur_thread);
    } else {
        cur_thread->esp = current_sp;
        cur_thread->status = RUNNABLE;
        sched_enqueue(cur_thread);
    }
    wake_up_threads();
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
    cur_thread = kmalloc(sizeof(Thread));
    cur_thread->id = 0;
    cur_thread->que.next = NULL;
    cur_thread->que.prev = NULL;
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

void sleep(size_t time)
{
    uint64_t wake_at = ticks + ((uint64_t)time * PIT_CRYSTAL);
    cur_thread->wake_at = wake_at;
    cur_thread->status = SLEEPING;
    asm __volatile__("int $0x81");
}
