#pragma once
#include "thread.h"

#define SLEEP_FOREVER SIZE_MAX
Thread *init_sched();
void sched_enqueue(Thread *thread);
Thread *spawn(void (*entry_point)(void *), void *arg);
void sched_sleep(size_t time);
