#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_check.h"
#include "hash_table.h"

#define HASH_MULTIPLIER_1 31
#define HASH_MULTIPLIER_2 17

static unsigned hash(const hash_table *table, const char *key);
static key_value_pair *lookup(const hash_table *table, const char *key);

hash_table *new_hash_table(int size) {
    hash_table *table = malloc(sizeof(hash_table));
    assert_malloc_success(table);
    table->size = size;
    table->pair_lists = malloc(size * sizeof(linked_list **));
    assert_malloc_success(table->pair_lists);
    return table;
}

static unsigned hash(const hash_table *table, const char *key) {
    unsigned hash_value;
    for (hash_value = 0; *key != '\0'; key++) {
        hash_value = *key * HASH_MULTIPLIER_1 + hash_value * HASH_MULTIPLIER_2;
    }
    return hash_value % table->size;
}

static key_value_pair *lookup(const hash_table *table, const char *key) {
    unsigned hash_value = hash(table, key);
    const linked_list *pair_list = table->pair_lists[hash_value];
    if (!pair_list) {
        return NULL;
    }
    const list_node *node = pair_list->head;
    while (node != NULL) {
        key_value_pair *pair = node->data;
        if (strcmp(pair->key, key) == EXIT_SUCCESS) {
            return pair;
        }
        node = list_get_next(node);
    }
    return NULL;
}

bool hash_table_contains(const hash_table *table, const char *key) {
    return lookup(table, key) != NULL;
}

void hash_table_add(const hash_table *table, const char *key, unsigned value) {
    key_value_pair *new_pair = malloc(sizeof(key_value_pair));
    assert_malloc_success(new_pair);
    new_pair->key = strdup(key);
    assert_malloc_success(new_pair->key);
    new_pair->value = value;
    unsigned hash_value = hash(table, key);
    linked_list **pair_list = &table->pair_lists[hash_value];
    if (!*pair_list) {
        *pair_list = new_linked_list();
    }
    list_append(*pair_list, new_pair);
}

unsigned hash_table_get(const hash_table *table, const char *key) {
    const key_value_pair *pair = lookup(table, key);
    if (!pair) {
        return -1;
    }
    return pair->value;
}

bool hash_table_set(const hash_table *table, const char *key,
        unsigned value) {
    key_value_pair *pair = lookup(table, key);
    if (pair == NULL) {
        return false;
    }
    pair->value = value;
    return true;
}

void hash_table_print(const hash_table *table) {
    for (int hash_value = 0; hash_value < table->size; hash_value++) {
        const linked_list *pair_list = table->pair_lists[hash_value];
        printf("%02d: ", hash_value);
        if (pair_list) {
            for (const list_node *node = table->pair_lists[hash_value]->head;
                    node != NULL; node = list_get_next(node)) {
                key_value_pair *pair = list_get_data(node);
                printf("{%s: %d} -> ", pair->key, pair->value);
            }
        }
        printf("\n");
    }
}

void hash_table_dispose(hash_table *table) {
    for (int hash_value = 0; hash_value < table->size; hash_value++) {
        linked_list *pair_list = table->pair_lists[hash_value];
        if (pair_list) {
            list_dispose(pair_list);
        }
    }
    free(table->pair_lists);
}

