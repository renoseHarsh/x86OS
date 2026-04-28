#pragma once

#include "thread.h"

void init_sched();
Thread *spawn(void (*entry)(void *), void *arg);
