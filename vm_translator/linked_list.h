#ifndef LINKED_LIST_H
#define LINKED_LIST_H

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
void list_print(linked_list *list);
void list_dispose(linked_list *list);

#endif

