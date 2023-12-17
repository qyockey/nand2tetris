#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

typedef struct list_node {
    void *data;
    struct list_node *next;
} list_node;

typedef struct {
    list_node *head;
    list_node *tail;
} linked_list;

linked_list *new_linked_list();
void list_append(linked_list *list, void *data);
void *list_get_data(const list_node *node);
list_node *list_get_next(const list_node *node);
bool list_contains(const linked_list *list, const void *data,
        bool (comparison) (const void *, const void *));
void *list_search(const linked_list *list, const void *data,
        bool (comparison) (const void *, const void *));
void list_print(const linked_list *list, const char *(to_string) (const void *));
void list_dispose(linked_list *list);

#endif

