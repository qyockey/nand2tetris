#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

void remove_spaces(char *asm_line);
void remove_comments(char *asm_line);

char *get_line(char *asm_line, int max_line, FILE *asm_file) {
  do {
    if (fgets(asm_line, max_line, asm_file) == NULL) {
      return NULL;
    } 
    remove_comments(asm_line);
    remove_spaces(asm_line);
  } while (strlen(asm_line) == 0);
  return asm_line;
}

bool is_label(char *asm_line) {
  return *asm_line == '(';
}

char *get_label(char *asm_line) {
  asm_line++;
  char *right_paren_index = strchr(asm_line, ')');
  *right_paren_index = '\0';
  return asm_line;
}

bool is_a_command(char *asm_line) {
  return *asm_line == '@';
}

void remove_spaces(char *asm_line) {
  char *no_spaces = asm_line;
  char c;
  while ((c = *asm_line) != '\0') {
    if (!isspace(c)) {
      *no_spaces++ = c;
    }
    asm_line++;
  }
  *no_spaces = '\0';
}

void remove_comments(char *asm_line) {
  char *p_inline = strstr(asm_line, "//");
  if (p_inline) {
    *p_inline = '\0';
  }
}

