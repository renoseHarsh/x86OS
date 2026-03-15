#include "node.h"
#include <stddef.h>

void push_node(Node **head_ref, Node *node)
{
    node->next = *head_ref;
    node->prev = NULL;

    if (*head_ref) {
        (*head_ref)->prev = node;
    }
    *head_ref = node;
}

void remove_node(Node **head_ref, Node *node)
{
    if (node == *head_ref) {
        pop_node(head_ref);
        return;
    }

    Node *next = node->next;
    Node *prev = node->prev;

    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;

    node->next = NULL;
    node->prev = NULL;
}

Node *pop_node(Node **head_ref)
{
    if (!head_ref) {
        return NULL;
    }
    Node *top = *head_ref;
    *head_ref = (*head_ref)->next;
    if (*head_ref) {
        (*head_ref)->prev = NULL;
    }
    return top;
}

void insert_node(Node *prev, Node *node)
{
    Node *next = prev->next;

    prev->next = node;
    node->prev = prev;

    if (next) {
        node->next = next;
        next->prev = node;
    }
}
