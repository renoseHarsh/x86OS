#include "dlist.h"
#include "process.h"
#include "reaper.h"
#include "sched.h"
#include "thread.h"

extern pde_t kernel_page_directory[];

static DList zombie_que;
Thread *reaper_thread;
void (*on_thread_exit)(Thread *) = NULL;

static Thread *reaper_pop()
{
    Thread *zombie = (Thread *)zombie_que.head;
    if (zombie)
        list_pop_front(&zombie_que);
    return zombie;
}

void reaper_add(Thread *t)
{
    t->status = ZOMBIE;
    asm __volatile__("cli");
    list_push_back(&zombie_que, (Node *)t);
    sched_enqueue(reaper_thread);
    asm __volatile__("sti");
}

static void destroy(Thread *zombie)
{
    if (zombie->pd == kernel_page_directory) {
        kdestroy_thread(zombie);
    } else
        destroy_process(zombie);
}

static void reaper_main()
{
    while (1) {
        __asm__ volatile("cli");
        Thread *zombie = reaper_pop();
        __asm__ volatile("sti");
        if (zombie) {
            destroy(zombie);
        } else
            sched_sleep(SLEEP_FOREVER);
    }
}

void init_reaper()
{
    zombie_que.head = NULL;
    zombie_que.tail = NULL;
    zombie_que.size = 0;
    on_thread_exit = reaper_add;
    reaper_thread = spawn((void *)reaper_main, NULL);
}
