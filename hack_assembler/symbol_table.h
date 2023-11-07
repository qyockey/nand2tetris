#include <stdbool.h>

void symbol_table_init();
bool symbol_table_contains(char *symbol);
void symbol_table_add(char *symbol, unsigned value);
unsigned symbol_table_get(char *symbol);
void symbol_table_print(void);

