#pragma once
#include "node.h"
#include <stddef.h>

typedef struct {
    Node *head;
    Node *tail;
    size_t size;
} DList;

void list_push_back(DList *list, Node *node);
void list_push_front(DList *list, Node *node);
void list_pop_front(DList *list);
void list_insert(Node *prev, Node *node);
void sorted_list_push(DList *list, Node *node);
