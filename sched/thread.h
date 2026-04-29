#pragma once

#include "node.h"
#include <stddef.h>
#include <stdint.h>

typedef enum { RUNNING, RUNNABLE, IDLE, ZOMBIE } THREAD_STATUS;

typedef struct {
    Node node;
    uint32_t esp;
    void *stack;
    THREAD_STATUS status;
    size_t id;
} Thread;

Thread *thread_create(void (*entry)(void *), void *arg);
