#pragma once
#include "thread.h"

void init_sched();
void sched_enqueue(Thread *thread);
void create_thread(void (*entry_point)(void *), void *arg);
