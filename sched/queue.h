#pragma once
#include "node.h"
#include <stddef.h>

typedef struct {
    Node *head;
    Node *tail;
    size_t size;
} Que;

void que_push(Que *que, Node *node);
void que_pop(Que *que);
