#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

static void remove_comments(const char *vm_line);
static void remove_newlines(char *vm_line);

char *get_line(char *vm_line, int max_line, FILE *vm_file) {
  do {
    if (fgets(vm_line, max_line, vm_file) == NULL) {
      return NULL;
    } 
    remove_comments(vm_line);
    remove_newlines(vm_line);
  } while (strlen(vm_line) == 0);
  return vm_line;
}

static void remove_comments(const char *vm_line) {
  char *p_inline = strstr(vm_line, "//");
  if (p_inline) {
    *p_inline = '\0';
  }
}

static void remove_newlines(char *vm_line) {
  while ((*vm_line) != '\0') {
    if (*vm_line == '\r' || *vm_line == '\n') {
      *vm_line = '\0';
    }
    vm_line++;
  }
}

