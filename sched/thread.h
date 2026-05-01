#pragma once

#include "node.h"
#include "paging.h"
#include <stddef.h>
#include <stdint.h>

typedef enum { RUNNING, RUNNABLE, IDLE, ZOMBIE, SLEEPING } THREAD_STATUS;

typedef struct {
    Node node;
    uint32_t esp;
    void *stack;
    THREAD_STATUS status;
    size_t id;
    uint64_t wake_at;
    pde_t *pd;
} Thread;

Thread *thread_create(void (*entry)(void *), void *arg);
void destroy_thread(Thread *thread);
