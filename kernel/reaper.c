#include "atomic.h"
#include "queue.h"
#include "reaper.h"
#include "sched.h"
#include "thread.h"
#include <stdbool.h>
#include <stdint.h>

extern void (*yield)();

void (*on_thread_exit)(Thread *);

static Thread *reaper;
static DLList zombie_que;

static void reaper_main(void *_)
{
    while (true) {
        ATOMIC_START();
        if (zombie_que.head) {
            ATOMIC_END();
            destroy_thread((Thread *)list_pop_front(&zombie_que));
        } else {
            reaper->status = SLEEPING;
            ATOMIC_END();
            yield();
        }
    }
}

void reaper_add(Thread *zombie)
{
    ATOMIC_START();
    zombie->status = ZOMBIE;
    list_push_back(&zombie_que, (Node *)zombie);
    if (reaper->status != RUNNABLE)
        thread_ready(reaper);
    ATOMIC_END();
}

void init_reaper()
{
    zombie_que.head = NULL;
    zombie_que.tail = NULL;
    zombie_que.size = 0;
    on_thread_exit = reaper_add;
    reaper = spawn(reaper_main, NULL);
}
