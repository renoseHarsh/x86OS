#pragma once
#include "node.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    RUNNABLE,  // Ready to run (waiting for CPU time)
    RUNNING,   // Currently executing on a CPU core
    TERMINATED // Execution finished or aborted
} ThreadStatus;

typedef struct {
    Node que;
    size_t id;
    ThreadStatus status;
    uintptr_t stack;
    uintptr_t esp;
} Thread;

void create_stack(void (*entry_point)(void *), void *arg, Thread *thread);
void free_stack(Thread *thread);
