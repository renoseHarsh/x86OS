#pragma once

#include "thread.h"

Thread *init_sched();
Thread *spawn(void (*entry)(void *), void *arg);
