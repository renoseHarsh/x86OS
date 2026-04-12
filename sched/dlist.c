#include "dlist.h"
#include "thread.h"
#include <stddef.h>

void list_pop_front(DList *list)
{
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
        return;
    }

    Node *next = list->head->next;
    list->head->next = NULL;
    next->prev = NULL;
    list->head = next;
    list->size--;
}

void list_push_front(DList *list, Node *node)
{
    list->size++;
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    }
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
    list->size++;
}

void list_push_back(DList *list, Node *node)
{
    list->size++;
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
    list->size++;
}

void list_insert(Node *prev, Node *node)
{
    Node *next = prev->next;
    prev->next = node;
    node->prev = prev;
    if (next)
        next->prev = node;
    node->next = next;
}
