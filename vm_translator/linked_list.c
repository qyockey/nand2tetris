/*******************************************************************************
** Program Filename: linked_list.c
** Author: Quinn Yockey
** Date: November 2023
** Description: A collection of basic functions to manipulate singly-linked
**     lists of any data type
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_check.h"
#include "linked_list.h"

/*******************************************************************************
** Function: new_linked_list
** Description: Allocates memory for list head, returns pointer to list
** Pre-Conditions: N/A
** Post-Conditions: Return value is non-null
*******************************************************************************/
linked_list *new_linked_list() {
    linked_list *list = (linked_list *) safe_malloc(sizeof(linked_list));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

/*******************************************************************************
** Function: list_append
** Description: Creates a new node at the end of list containing data
** Parameters:
**     - list: pointer to linked list to append data to
**     - data: data to append
** Pre-Conditions: list is non-null
** Post-Conditions:
**     - list is non-null
**     - list->head and list->tail are both non-null
**     - list->tail->data is set to data
**     - list->tail->next is null
*******************************************************************************/
void list_append(linked_list *list, void *data) {
    list_node *new_tail = (list_node *) safe_malloc(sizeof(list_node));
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

/*******************************************************************************
** Function: list_get_data
** Description: Returns the data contained in node
** Parameters:
**     - node: List node to get data from
** Pre-Conditions: node is non-null 
** Post-Conditions: N/A
*******************************************************************************/
void *list_get_data(const list_node *node) {
    assert_nonnull(node, "Error: Cannot get data from NULL list node\n");
    return node->data;
}

/*******************************************************************************
** Function: list_get_next
** Description: Returns a pointer to the next node in the list
** Parameters:
**     - node: List node to get next from
** Pre-Conditions: node is non-null 
** Post-Conditions: N/A
*******************************************************************************/
list_node *list_get_next(const list_node *node) {
    assert_nonnull(node, "Error: Cannot get data from NULL list node\n");
    return node->next;
}

/*******************************************************************************
** Function: list_contains
** Description: Returns true if list contains data, false if it doesn't
** Parameters:
**     - list: List to search
**     - data: Data to search for
**     - comparison: Function to use to test equality
** Pre-Conditions: list is non-null
** Post-Conditions: N/A
*******************************************************************************/
bool list_contains(const linked_list *list, const void *data,
        bool (comparison) (const void *, const void *)) {
    return list_search(list, data, comparison) != NULL;
}

/*******************************************************************************
** Function: list_search
** Description: Returns the first list node data which passes comparison, or
**     NULL if none pass
** Parameters:
**     - list: List to search
**     - data: Data to search for
**     - comparison: Function to use to test equality
** Pre-Conditions: list is non-null
** Post-Conditions: N/A
*******************************************************************************/
void *list_search(const linked_list *list, const void *data,
        bool (comparison) (const void *, const void *)) {
    assert_nonnull(list, "Error: Cannot search NULL list\n");
    for (list_node *node = list->head; node != NULL; node = node->next) {
        if (comparison(node->data, data)) {
            // match found
            return node->data;
        }
    }
    // no match
    return NULL;
}

/*******************************************************************************
** Function: list_print
** Description: Print non-null list nodes, separated by "->"
** Parameters:
**     - list: List to print
**     - to_string: Function to get string representation of data
** Pre-Conditions: list is non-null
** Post-Conditions: N/A
*******************************************************************************/
void list_print(const linked_list *list, const char *(to_string) (const void *)) {
    assert_nonnull(list, "Error: Cannot print NULL list\n");
    for (list_node *node = list->head; node != NULL; node = node->next) {
        printf("%s", to_string(node->data));
        if (node->next) {
            printf("->");
        }
    }
    printf("\n");
}

/*******************************************************************************
** Function: list_dispose
** Description: Free all memory allocated to list
** Parameters:
**     - list: Linked list to dispose
**     - free_list_data: Whether to free data inside list nodes
** Pre-Conditions: list is non-null
** Post-Conditions: The memory allocated to data of all nodes (if applicable),
**     the nodes themselves, and the list is all freed
*******************************************************************************/
void list_dispose(linked_list *list) {
    assert_nonnull(list, "Error: Cannot free memory from NULL list");
    list_node *node = list->head;
    while (node) {
        list_node *next = node->next;
        free(node->data);
        free(node);
        node = next;
    }
    free(list);
}

