#include "atomic.h"
#include "heap/heap.h"
#include "kernel.h"
#include "queue.h"
#include "sched.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

extern void context_switch(uint32_t *old_esp, uint32_t *new_esp);
extern uint64_t ticks;

void (*_yield)();

DLList que, sleep_que;
Thread *cur_thread, *idle_thread;

static void wake_up_threads()
{
    while (sleep_que.head && ((Thread *)sleep_que.head)->wake_at <= ticks) {
        Thread *thread = (Thread *)list_pop_front(&sleep_que);
        thread->status = RUNNABLE;
        list_push_back(&que, (Node *)thread);
    }
}

static Thread *get_next_thread()
{
    if (que.head) {
        Thread *thread = (Thread *)list_pop_front(&que);
        return thread;
    }
    return NULL;
}

static void scheduler()
{
    ATOMIC_START();
    if (cur_thread->status == RUNNING) {
        cur_thread->status = RUNNABLE;
        list_push_back(&que, (Node *)cur_thread);
    }
    wake_up_threads();
    Thread *next_thread = get_next_thread() ?: idle_thread;
    Thread *prev_thread = cur_thread;
    cur_thread = next_thread;
    if (cur_thread != idle_thread)
        cur_thread->status = RUNNING;
    context_switch(&prev_thread->esp, &cur_thread->esp);
    ATOMIC_END();
}

Thread *spawn(void (*entry)(void *), void *arg)
{
    Thread *thread = thread_create(entry, arg);
    thread->status = RUNNABLE;
    ATOMIC_START();
    list_push_back(&que, (Node *)thread);
    ATOMIC_END();
    return thread;
}

Thread *init_sched()
{
    que.size = 0;
    que.head = NULL;
    que.tail = NULL;

    sleep_que.size = 0;
    sleep_que.head = NULL;
    sleep_que.tail = NULL;

    cur_thread = kmalloc(sizeof(Thread));
    _yield = scheduler;
    idle_thread = cur_thread;
    cur_thread->id = 0;
    return cur_thread;
}

void thread_sleep(size_t time)
{

    uint64_t wake_at = ticks + ((uint64_t)time * PIT_CRYSTAL);
    cur_thread->wake_at = wake_at;
    ATOMIC_START();
    cur_thread->status = SLEEPING;
    list_sorted_push(&sleep_que, (Node *)cur_thread);
    ATOMIC_END();
    yield();
}
