#include <stdbool.h>

char *get_line(char *asm_line, int max_line, FILE *asm_file);
bool is_label(char *asm_line);
char *get_label(char *asm_line);
bool is_a_command(char *command);

