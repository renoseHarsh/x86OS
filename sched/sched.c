#include "heap/heap.h"
#include "queue.h"
#include "sched.h"
#include "thread.h"

extern void context_switch(uint32_t *old_esp, uint32_t *new_esp);

void (*yield)();

DLList que;
Thread *cur_thread;

static Thread *get_next_thread()
{
    if (que.head)
        return (Thread *)list_pop_front(&que);
    return NULL;
}

static void scheduler()
{
    list_push_back(&que, (Node *)cur_thread);
    Thread *next_thread = get_next_thread();
    Thread *prev_thread = cur_thread;
    cur_thread = next_thread;
    context_switch(&prev_thread->esp, &cur_thread->esp);
}

Thread *spawn(void (*entry)(void *), void *arg)
{
    Thread *thread = thread_create(entry, arg);
    list_push_back(&que, (Node *)thread);
    return thread;
}

void init_sched()
{
    que.size = 0;
    que.head = NULL;
    que.tail = NULL;
    cur_thread = kmalloc(sizeof(Thread));
    yield = scheduler;
}
