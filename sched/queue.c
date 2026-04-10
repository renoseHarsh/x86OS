#include "queue.h"
#include <stddef.h>

void que_pop(Que *que)
{
    if (que->head == que->tail) {
        que->head = NULL;
        que->tail = NULL;
        return;
    }

    Node *next = que->head->next;
    que->head->next = NULL;
    next->prev = NULL;
    que->head = next;
}

void que_push(Que *que, Node *node)
{
    que->size++;
    if (que->head == NULL) {
        que->head = node;
        que->tail = node;
        return;
    }
    node->prev = que->tail;
    que->tail->next = node;
    que->tail = node;
}
