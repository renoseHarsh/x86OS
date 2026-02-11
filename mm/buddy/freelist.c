#include "freelist.h"
#include <stddef.h>

void push_node(Page **head_ref, Page *node)
{
    node->next = *head_ref;
    node->prev = NULL;

    if (*head_ref) {
        (*head_ref)->prev = node;
    }
    *head_ref = node;
}

void remove_node(Page **head_ref, Page *node)
{
    if (node == *head_ref) {
        pop_node(head_ref);
        return;
    }
    Page *next = node->next;
    Page *prev = node->prev;

    if (next)
        next->prev = prev;
    if (next)
        next->next = next;
}

Page *pop_node(Page **head_ref)
{
    if (!head_ref) {
        return NULL;
    }
    Page *top = *head_ref;
    *head_ref = (*head_ref)->next;
    if (*head_ref) {
        (*head_ref)->prev = NULL;
    }
    return top;
}
