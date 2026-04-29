#include "atomic.h"
#include "heap/heap.h"
#include "queue.h"
#include "sched.h"
#include "thread.h"

extern void context_switch(uint32_t *old_esp, uint32_t *new_esp);

void (*yield)();

DLList que;
Thread *cur_thread, *idle_thread;

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
    cur_thread = kmalloc(sizeof(Thread));
    yield = scheduler;
    idle_thread = cur_thread;
    cur_thread->id = 0;
    return cur_thread;
}
