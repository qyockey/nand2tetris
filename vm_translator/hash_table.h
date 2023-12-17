#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>

#include "linked_list.h"

typedef struct {
    char *key;
    void *value;
} key_value_pair;

typedef struct {
    linked_list **pair_lists;
    int size;
} hash_table; 

hash_table *hash_table_init(unsigned size);
bool hash_table_contains(const hash_table *table, const char *key);
void hash_table_add(const hash_table *table, const char *key, void *value);
void *hash_table_get(const hash_table *table, const char *key);
bool hash_table_set(const hash_table *table, const char *key, void *value);
void hash_table_print(const hash_table *table);
void hash_table_dispose(hash_table *table);

#endif

