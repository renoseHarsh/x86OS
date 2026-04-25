#pragma once
#include "node.h"
#include <stddef.h>
#include <stdint.h>

typedef enum { RUNNABLE, RUNNING, SLEEPING, TERMINATED } ThreadStatus;

typedef struct {
    Node que;
    size_t id;
    ThreadStatus status;
    uintptr_t stack;
    uintptr_t esp;
    uint64_t wake_at;
} Thread;

Thread *create_thread(void (*entry_point)(void *), void *arg);
void destroy_thread(Thread *thread);
