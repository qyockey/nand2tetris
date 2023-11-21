#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node {
  void *data;
  struct node *next;
} node;

node *list_append(node *tail, void *data);
void list_dispose(node *head);

#endif

