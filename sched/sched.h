#pragma once
#include "thread.h"
#include <stddef.h>

void init_sched();
void sched_enqueue(Thread *thread);
void spawn(void (*entry_point)(void *), void *arg);
void sched_sleep(size_t time);
