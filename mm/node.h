#pragma once

typedef struct Node {
    struct Node *next;
    struct Node *prev;
} Node;

void push_node(Node **head_ref, Node *node);
void remove_node(Node **head_ref, Node *node);
Node *pop_node(Node **head_ref);
