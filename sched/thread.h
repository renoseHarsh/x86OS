#pragma once
#include "node.h"
#include "paging.h"
#include <stddef.h>
#include <stdint.h>

typedef enum { RUNNABLE, RUNNING, SLEEPING, TERMINATED } ThreadStatus;

typedef struct {
    Node que;
    size_t id;
    ThreadStatus status;
    uint64_t wake_at;
    uintptr_t kernel_stack_base;
    uintptr_t kernel_esp;
    pde_t *pd;
} Thread;

Thread *kcreate_thread(void (*entry_point)(void *), void *arg);
void kdestroy_thread(Thread *thread);
