#pragma once

#include "node.h"
#include <stdint.h>

typedef struct {
    Node node;
    uint32_t esp;
    void *stack;
} Thread;

Thread *thread_create(void (*entry)(void *), void *arg);
