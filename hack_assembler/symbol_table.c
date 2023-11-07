#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

#define TABLE_SIZE 100

typedef struct node {
  char *symbol;
  unsigned value;
  struct node *next;
} Entry;

static Entry *symbol_table[TABLE_SIZE];

static unsigned hash(char *symbol) {
  unsigned hash_val;
  for (hash_val = 0; *symbol != '\0'; symbol++) {
    hash_val = *symbol + 31 * hash_val;
  }
  return hash_val % TABLE_SIZE;
}

static Entry *lookup(char *symbol) {
  unsigned hash_val = hash(symbol);
  Entry *entry = symbol_table[hash_val];
  while (entry != NULL) {
    if (strcmp(entry->symbol, symbol) == 0) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

void symbol_table_init() {
  symbol_table_add("R0", 0);
  symbol_table_add("R1", 1);
  symbol_table_add("R2", 2);
  symbol_table_add("R3", 3);
  symbol_table_add("R4", 4);
  symbol_table_add("R5", 5);
  symbol_table_add("R6", 6);
  symbol_table_add("R7", 7);
  symbol_table_add("R8", 8);
  symbol_table_add("R9", 9);
  symbol_table_add("R10", 10);
  symbol_table_add("R11", 11);
  symbol_table_add("R12", 12);
  symbol_table_add("R13", 13);
  symbol_table_add("R14", 14);
  symbol_table_add("R15", 15);
  symbol_table_add("SCREEN", 16384);
  symbol_table_add("KBD", 24576);
  symbol_table_add("SP", 0);
  symbol_table_add("LCL", 1);
  symbol_table_add("ARG", 2);
  symbol_table_add("THIS", 3);
  symbol_table_add("THAT", 4);
}

bool symbol_table_contains(char *symbol) {
  return lookup(symbol) != NULL;
}

void symbol_table_add(char *symbol, unsigned value) {
  Entry *new_entry = (Entry *) malloc(sizeof(Entry));
  if (new_entry == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  new_entry->symbol = (char *) malloc((strlen(symbol) + 1) * sizeof(char));
  if (new_entry->symbol == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(new_entry->symbol, symbol);
  new_entry->value = value;
  new_entry->next = NULL;
  unsigned hash_val = hash(symbol);
  Entry *entry = symbol_table[hash_val];
  if (entry == NULL) {
    symbol_table[hash_val] = new_entry;
  } else {
    while (entry->next != NULL) {
      entry = entry->next;
    }
    entry->next = new_entry;
  }
}

unsigned symbol_table_get(char *symbol) {
  Entry *entry = lookup(symbol);
  if (entry == NULL) {
    return -1;
  }
  return entry->value;
}

void symbol_table_print(void) {
  for (int hash_val = 0; hash_val < TABLE_SIZE; hash_val++) {
    printf("%d: ", hash_val);
    for (Entry *entry = symbol_table[hash_val]; entry != NULL; entry = entry->next) {
      printf("{%s: %d} -> ", entry->symbol, entry->value);
    }
    printf("\n");
  }
}

