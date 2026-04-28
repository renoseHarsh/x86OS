#include "heap/heap.h"
#include "sched.h"
#include "thread.h"
#include <stdint.h>

extern void context_switch(uint32_t *old_esp, uint32_t new_esp);

void (*yield)();

Thread *cur, *next;

static void scheduler()
{
    Thread *temp = next;
    next = cur;
    cur = temp;
    context_switch(&next->esp, cur->esp);
}

void init_sched(Thread *thread)
{
    yield = scheduler;
    cur = kmalloc(sizeof(Thread));
    next = thread;
}
