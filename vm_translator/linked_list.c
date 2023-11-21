#include <stdio.h>
#include <stdlib.h>

#include "error_check.h"
#include "linked_list.h"

node *list_append(node *tail, void *data) {
  node *new_tail = (node *) malloc(sizeof(node));
  assert_malloc_success((void *) new_tail);
  new_tail->data = data;
  new_tail->next = NULL;
  if (tail != NULL) {
    tail->next = new_tail;
  }
  return new_tail;
}

void list_dispose(node *head) {
  node *next;
  do {
    next = head->next;
    free(head);
    head = next;
  } while (next != NULL);
}

