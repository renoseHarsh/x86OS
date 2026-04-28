#include "panic.h"
#include "queue.h"

void list_push_back(DLList *list, Node *node)
{
    node->next = NULL;
    node->prev = list->tail;

    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
    list->size++;
}

Node *list_pop_front(DLList *list)
{
    if (list->head == NULL)
        kernel_panic("Underflow\n");
    Node *node = list->head;

    if (list->head != list->tail)
        node->next->prev = NULL;
    else
        list->tail = NULL;
    list->head = node->next;
    node->next = NULL;
    list->size--;
    return node;
}
void list_push_front(DLList *list, Node *node)
{
    node->prev = NULL;
    node->next = list->head;

    if (list->head)
        list->head->prev = node;
    else
        list->tail = node;
    list->head = node;
    list->size++;
}

void list_sorted_push(DLList *list, Node *node)
{
    // empty push front
    // find slot the first element that is bigger than me name it x
    // if x is head then push front
    // if x was never found then push back
    // else insert node before x

    // Node *cur = list->head;
    // while (cur && ((Thread *)cur)->wake_at <= ((Thread *)node)->wake_at)
    //     cur = cur->next;
    //
    // if (cur == NULL)
    //     list_push_back(list, node);
    // else if (cur == list->head)
    //     list_push_front(list, node);
    // else {
    //     node->prev = cur->prev;
    //     node->next = cur;
    //     cur->prev->next = node;
    //     cur->prev = node;
    //     list->size++;
    // }
}
