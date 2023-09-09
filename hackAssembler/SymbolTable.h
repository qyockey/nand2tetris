#include <stdbool.h>

void symbolTableInit();
bool symbolTableContains(char *symbol);
void symbolTableAdd(char *symbol, unsigned value);
unsigned symbolTableGet(char *symbol);
void symbolTablePrint(void);
