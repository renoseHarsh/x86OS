#include "atomic.h"
#include "gdt.h"
#include "heap/heap.h"
#include "pit.h"
#include "queue.h"
#include "sched.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

extern void context_switch(uint32_t *old_esp, uint32_t *new_esp);
extern TSS_ENTRY tss;
extern pde_t kernel_page_directory[];

static uint64_t ticks;
static uint16_t tick_rate;

DLList que, sleep_que;
Thread *cur_thread, *idle_thread;

static void _thread_ready(Thread *thread)
{
    thread->status = RUNNABLE;
    list_push_back(&que, (Node *)thread);
}

static void wake_up_threads()
{
    while (sleep_que.head && ((Thread *)sleep_que.head)->wake_at <= ticks) {
        Thread *thread = (Thread *)list_pop_front(&sleep_que);
        _thread_ready(thread);
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

void scheduler()
{
    ATOMIC_START();
    if (cur_thread->status == RUNNING) {
        _thread_ready(cur_thread);
    }
    Thread *prev_thread = cur_thread;
    cur_thread = get_next_thread();
    if (!cur_thread)
        cur_thread = idle_thread;
    if (cur_thread != idle_thread)
        cur_thread->status = RUNNING;
    if (cur_thread->pd != prev_thread->pd)
        refresh_cr3(cur_thread->pd);
    tss.esp0 = (uint32_t)cur_thread->stack + 0x1000;
    context_switch(&prev_thread->esp, &cur_thread->esp);
    ATOMIC_END();
}

static void timer()
{
    ticks += tick_rate;
    wake_up_threads();
    scheduler();
}

Thread *init_sched()
{
    tick_rate = init_pit(100, timer);
    que.size = 0;
    que.head = NULL;
    que.tail = NULL;

    sleep_que.size = 0;
    sleep_que.head = NULL;
    sleep_que.tail = NULL;

    cur_thread = kmalloc(sizeof(Thread));
    idle_thread = cur_thread;
    cur_thread->id = 0;
    cur_thread->pd = kernel_page_directory;
    return cur_thread;
}

void thread_sleep(size_t time)
{
    uint64_t wake_at = ticks + ((uint64_t)time * (uint64_t)PIT_CRYSTAL);
    cur_thread->wake_at = wake_at;
    ATOMIC_START();
    cur_thread->status = SLEEPING;
    list_sorted_push(&sleep_que, (Node *)cur_thread);
    ATOMIC_END();
    scheduler();
}

void thread_ready(Thread *thread)
{
    ATOMIC_START();
    thread->status = RUNNABLE;
    list_push_back(&que, (Node *)thread);
    ATOMIC_END();
}
