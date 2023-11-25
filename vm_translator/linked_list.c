#include <stdio.h>
#include <stdlib.h>

#include "error_check.h"
#include "linked_list.h"

linked_list *new_linked_list() {
    linked_list *list = (linked_list *) malloc(sizeof(linked_list));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_append(linked_list *list, void *data) {
    list_node *new_tail = (list_node *) malloc(sizeof(list_node));
    assert_malloc_success((void *) new_tail);
    new_tail->data = data;
    new_tail->next = NULL;
    if (list->tail) {
        list_node *old_tail = list->tail;
        old_tail->next = new_tail;
    }
    if (!list->head) {
        list->head = new_tail;
    }
    list->tail = new_tail;
}

void *list_get_data(const list_node *node) {
    return node->data;
}

list_node *list_get_next(const list_node *node) {
    return node->next;
}

void list_print(linked_list *list) {
    for (list_node *node = list->head; node != NULL; node = node->next) {
        printf("%d->", *((int *) node->data));
    }
    printf("\b\b  \n");
}

void list_dispose(linked_list *list) {
    list_node *node = list->head;
    while (node != NULL) {
        list_node *next = node->next;
        free(node);
        node = next;
    }
}

