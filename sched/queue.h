#pragma once

#include "node.h"
#include <stddef.h>

typedef struct {
    Node *head;
    Node *tail;
    size_t size;
} DLList;

void list_push_back(DLList *list, Node *node);
Node *list_pop_front(DLList *list);
void list_push_front(DLList *list, Node *node);
void list_sorted_push(DLList *list, Node *node);
