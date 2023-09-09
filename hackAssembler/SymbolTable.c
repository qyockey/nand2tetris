#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SymbolTable.h"

#define TABLE_SIZE 100

typedef struct node {
  char *symbol;
  unsigned value;
  struct node *next;
} Entry;

static Entry *symbolTable[TABLE_SIZE];

static unsigned hash(char *symbol) {
  unsigned hashVal;
  for (hashVal = 0; *symbol != '\0'; symbol++) {
    hashVal = *symbol + 31 * hashVal;
  }
  return hashVal % TABLE_SIZE;
}

static Entry *lookup(char *symbol) {
  unsigned hashVal = hash(symbol);
  Entry *entry = symbolTable[hashVal];
  while (entry != NULL) {
    if (strcmp(entry->symbol, symbol) == 0) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

void symbolTableInit() {
  symbolTableAdd("R0", 0);
  symbolTableAdd("R1", 1);
  symbolTableAdd("R2", 2);
  symbolTableAdd("R3", 3);
  symbolTableAdd("R4", 4);
  symbolTableAdd("R5", 5);
  symbolTableAdd("R6", 6);
  symbolTableAdd("R7", 7);
  symbolTableAdd("R8", 8);
  symbolTableAdd("R9", 9);
  symbolTableAdd("R10", 10);
  symbolTableAdd("R11", 11);
  symbolTableAdd("R12", 12);
  symbolTableAdd("R13", 13);
  symbolTableAdd("R14", 14);
  symbolTableAdd("R15", 15);
  symbolTableAdd("SCREEN", 16384);
  symbolTableAdd("KBD", 24576);
  symbolTableAdd("SP", 0);
  symbolTableAdd("LCL", 1);
  symbolTableAdd("ARG", 2);
  symbolTableAdd("THIS", 3);
  symbolTableAdd("THAT", 4);
}

bool symbolTableContains(char *symbol) {
  return lookup(symbol) != NULL;
}

void symbolTableAdd(char *symbol, unsigned value) {
  Entry *newEntry = (Entry *) malloc(sizeof(Entry));
  if (newEntry == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  newEntry->symbol = (char *) malloc((strlen(symbol) + 1) * sizeof(char));
  if (newEntry->symbol == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(newEntry->symbol, symbol);
  newEntry->value = value;
  newEntry->next = NULL;
  unsigned hashVal = hash(symbol);
  Entry *entry = symbolTable[hashVal];
  if (entry == NULL) {
    symbolTable[hashVal] = newEntry;
  } else {
    while (entry->next != NULL) {
      entry = entry->next;
    }
    entry->next = newEntry;
  }
}

unsigned symbolTableGet(char *symbol) {
  Entry *entry = lookup(symbol);
  if (entry == NULL) {
    return -1;
  }
  return entry->value;
}

void symbolTablePrint(void) {
  for (int hashVal = 0; hashVal < TABLE_SIZE; hashVal++) {
    printf("%d: ", hashVal);
    for (Entry *entry = symbolTable[hashVal]; entry != NULL; entry = entry->next) {
      printf("{%s: %d} -> ", entry->symbol, entry->value);
    }
    printf("\n");
  }
}
