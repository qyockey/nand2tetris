/*******************************************************************************
** Program Filename: hash_table.c
** Author: Quinn Yockey
** Date: November 2023
** Description: A collection of basic functions to manipulate hash tables with
**     string-int key-value pairs.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_check.h"
#include "hash_table.h"

#define HASH_MULTIPLIER_1 31
#define HASH_MULTIPLIER_2 17
#define KEY_VALUE_PAIR_MAX_STR_LEN 128

static unsigned hash(const char *key, unsigned table_size);
static key_value_pair *lookup(const hash_table *table, const char *key);
static const char *key_value_pair_to_string(const void *pair);
static bool key_value_pair_has_key(const void *pair, const void *key);

/*******************************************************************************
** Function: hash_table_init
** Description: Allocates memory for a hash_table and returns a pointer to the
**     table. The table contains an array of linked lists to prevent collisions.
** Parameters:
**     - size: Number of linked lists in array
** Pre-Conditions: N/A
** Post-Conditions: Return value is a non-null pointer to an initialized hash
**     table. Linked lists are not initialized.
*******************************************************************************/
hash_table *hash_table_init(unsigned size) {
    hash_table *table = safe_malloc(sizeof(hash_table));
    table->size = size;
    table->pair_lists = safe_malloc(size * sizeof(linked_list **));
    for (int list_id = 0; list_id < size; list_id++) {
        table->pair_lists[list_id] = NULL;
    }
    return table;
}

/*******************************************************************************
** Function: hash
** Description: Returns a hash value in the range [0, table_size) using the
**     polynomial rolling hash algorithm. It's nowrhere near perfect or secure
**     but it's quick and more than sufficient for this project
** Parameters:
**     - key: String to hash
**     - table_size: Upper bound for range of hash function
** Pre-Conditions: key is non-null
** Post-Conditions: Return value is in range [0, table_size)
*******************************************************************************/
static unsigned hash(const char *key, unsigned table_size) {
    assert_nonnull(key, "Error: Cannot hash a NULL string\n");
    unsigned hash_value;
    for (hash_value = 0; *key != '\0'; key++) {
        hash_value = *key * HASH_MULTIPLIER_1 + hash_value * HASH_MULTIPLIER_2;
    }
    return hash_value % table_size;
}

/*******************************************************************************
** Function: lookup
** Description: Returns a key_value_pair in table corresponding to key, or NULL
**     if key is not contained within table.
** Parameters:
**     - table: hash_table to search in
**     - key: String to get key_value_pair for
** Pre-Conditions: table and key are both non-null
** Post-Conditions: N/A
*******************************************************************************/
static key_value_pair *lookup(const hash_table *table, const char *key) {
    assert_nonnull(table, "Error: Cannot lookup key in NULL table\n");
    assert_nonnull(key, "Error: Cannot lookup NULL key in table\n");
    unsigned hash_value = hash(key, table->size);
    const linked_list *pair_list = table->pair_lists[hash_value];
    if (!pair_list) {
        // no keys with matching hash
        return NULL;
    }
    return list_search(pair_list, key, &key_value_pair_has_key);
}

/*******************************************************************************
** Function: hash_table_contains
** Description: Returns whether or not table contains key
** Parameters:
**     - table: hash_table to search in
**     - key: String to search for
** Pre-Conditions: table and key are both non-null
** Post-Conditions: N/A
*******************************************************************************/
bool hash_table_contains(const hash_table *table, const char *key) {
    return lookup(table, key) != NULL;
}

/*******************************************************************************
** Function: hash_table_add
** Description: Add a key-value pair to table
** Parameters:
**     - table: hash_table to put key_value_pair in
**     - key: key of key_value_pair
**     - value: value of key_value_pair
** Pre-Conditions: table and key are both non-null
** Post-Conditions: A key_value_pair containing key and value was appended to
**     the key-value pair linked list matching the hash value of key
*******************************************************************************/
void hash_table_add(const hash_table *table, const char *key, unsigned value) {
    assert_nonnull(table, "Error: Cannot add key-value pair to "
            "NULL hash table\n");
    assert_nonnull(key, "Error: Cannot add NULL key to key-value pair "
            "in hash table\n");
    assert_condition(!hash_table_contains(table, key), "Error: Key already "
            "exists in hash table, cannot add it again\n");
    key_value_pair *new_pair = safe_malloc(sizeof(key_value_pair));
    new_pair->key = safe_strdup(key);
    new_pair->value = value;
    unsigned hash_value = hash(key, table->size);
    linked_list **pair_list = &table->pair_lists[hash_value];
    if (!*pair_list) {
        *pair_list = new_linked_list();
    }
    list_append(*pair_list, new_pair);
}

/*******************************************************************************
** Function: hash_table_get
** Description: Returns value mapped to key, or -1 (UINT_MAX) if key not found
** Parameters:
**     - table: hash_table to search for key in
**     - key: key to search for
** Pre-Conditions: table and key are both non-null
** Post-Conditions: A key_value_pair containing key and value was appended to
**     the key-value pair linked list matching the hash value of key
*******************************************************************************/
unsigned hash_table_get(const hash_table *table, const char *key) {
    assert_nonnull(table, "Error: Cannot search NULL hash table\n");
    assert_nonnull(key, "Error: Cannot search for NULL key in hash table\n");
    const key_value_pair *pair = lookup(table, key);
    if (!pair) {
        // key not found
        return -1;
    }
    return pair->value;
}

/*******************************************************************************
** Function: hash_table_set
** Description: Sets finds key value pair matching key and sets value to value.
**     Returns true if succesful or false if key wasn't found
** Parameters:
**     - table: hash_table to search for key in
**     - key: Key to search for
**     - value: Value to set value in key-value pair equal to
** Pre-Conditions: table and key are both non-null
** Post-Conditions: The key value pair whose key matches the provided key has 
**     its value set to the provided value
*******************************************************************************/
bool hash_table_set(const hash_table *table, const char *key, unsigned value) {
    key_value_pair *pair = lookup(table, key);
    if (!pair) {
        return false;
    }
    pair->value = value;
    return true;
}

/*******************************************************************************
** Function: hash_table_print
** Description: Prints each linked list within hash table to show collisions
** Parameters:
**     - table: hash_table to print
** Pre-Conditions: table is non-null
** Post-Conditions: N/A
*******************************************************************************/
void hash_table_print(const hash_table *table) {
    assert_nonnull(table, "Error: Cannot print NULL hash table\n");
    for (int hash_value = 0; hash_value < table->size; hash_value++) {
        const linked_list *pair_list = table->pair_lists[hash_value];
        printf("%02d: ", hash_value);
        if (pair_list) {
            list_print(pair_list, &key_value_pair_to_string);
        }
        printf("\n");
    }
}

/*******************************************************************************
** Function: key_value_pair_to_string
** Description: Generates a string representation of a key_value_pair
** Parameters:
**     - pair: key_value_pair to create string of
** Pre-Conditions: If non-null, key and value can fit into
**     KEY_VALUE_PAIR_MAX_STRING_LEN - 5 characters or less
** Post-Conditions: Return value is non-null
*******************************************************************************/
static const char *key_value_pair_to_string(const void *pair) {
    static char string[KEY_VALUE_PAIR_MAX_STR_LEN];
    if (!pair) {
        sprintf(string, "{NULL}");
        return string;
    }
    const key_value_pair *kv_pair = pair;
    const char *key = kv_pair->key;
    unsigned value = kv_pair->value;
    sprintf(string, "{%s: %d}", key, value);
    return string;
}


/*******************************************************************************
** Function: key_value_pair_has_key
** Description: Checks whether the key in a key_value_pair matches a
**     given string
** Parameters:
**     - pair: key_value_pair to check key of
**     - compare: String to check if equal to key
** Pre-Conditions: pair and compare are both non-null
** Post-Conditions: N/A
*******************************************************************************/
static bool key_value_pair_has_key(const void *pair, const void *compare) {
    assert_nonnull(pair, "Error: Cannot compare key of NULL key_value_pair\n");
    assert_nonnull(compare, "Error: Cannot compare key of key_value_pair"
            "to NULL string\n");
    const key_value_pair *kv_pair = pair;
    const char *key = kv_pair->key;
    return strcmp(key, compare) == EXIT_SUCCESS;
}

/*******************************************************************************
** Function: hash_table_dispose
** Description: Frees memory allocated to each linked list within table as well
**     the table itself
** Parameters:
**     - table: hash_table to free momory allocated to
** Pre-Conditions: table is non-null
** Post-Conditions: N/A
*******************************************************************************/
void hash_table_dispose(hash_table *table) {
    assert_nonnull(table, "Error: Cannot dispose NULL hash table\n");
    for (int hash_value = 0; hash_value < table->size; hash_value++) {
        linked_list *pair_list = table->pair_lists[hash_value];
        if (pair_list) {
            list_node *node = pair_list->head;
            while(node) {
                key_value_pair *pair = node->data;
                free(pair->key);
                free(pair);
                list_node *next = node->next;
                free(node);
                node = next;
            }
            free(pair_list);
        }
    }
    free(table->pair_lists);
    free(table);
}

