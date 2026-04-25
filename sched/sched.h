#pragma once
#include <stddef.h>

void init_sched();
void spawn(void (*entry_point)(void *), void *arg);
void sched_sleep(size_t time);
