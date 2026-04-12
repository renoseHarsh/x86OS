#pragma once
#include "thread.h"
#include <stddef.h>

void init_sched();
void sched_enqueue(Thread *thread);
void create_thread(void (*entry_point)(void *), void *arg);
void sleep(size_t time);
