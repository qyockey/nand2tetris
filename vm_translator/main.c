#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm_generator.h"
#include "parser.h"

#define VM_EXTENSION_LEN  3
#define ASM_EXTENSION_LEN 4
#define VM_EXTENSION  ".vm"
#define ASM_EXTENSION ".asm"
#define MAX_VM_LINE   80
#define MAX_ASM_LINE 160

static char *get_asm_filename(char *root_filename, int root_filename_len);

static char *root_filename;

int main(int argc, char *argv[]) {
  static char vm_line[MAX_VM_LINE];
  static char asm_line[MAX_ASM_LINE];
  static FILE *vm_file, *asm_file;

  if (argc < 2) {  // argv[0] = vm_emulator, argc[1] = file1.vm ...
    perror("usage: $ vm_emulator file1.vm [, file2.vm ...]");
  }
  for (int vm_file_index = 1; vm_file_index < argc; vm_file_index++) {
    char *vm_filename = argv[vm_file_index];
    int root_filename_len = strlen(vm_filename) - VM_EXTENSION_LEN; 
    if (strstr(vm_filename, VM_EXTENSION) - vm_filename != root_filename_len) {
      printf("error: %s not supported, must end in %s\n", vm_filename, VM_EXTENSION);
      continue;
    }
    vm_file = fopen(vm_filename, "r");
    if (vm_file == NULL) {
      printf("file not found: %s\n", vm_filename);
      exit(EXIT_FAILURE);
    }
    root_filename = (char *) malloc(root_filename_len * sizeof(char));
    if (root_filename == NULL) {
      printf("malloc error\n");
      exit(EXIT_FAILURE);
    }
    strncpy(root_filename, vm_filename, root_filename_len);
    char *asm_filename = get_asm_filename(root_filename, root_filename_len);
    asm_file = fopen(asm_filename, "w");
    if (asm_file == NULL) {
      printf("file not found: %s\n", asm_filename);
      exit(EXIT_FAILURE);
    }
    free(asm_filename);

    while ((get_line(vm_line, MAX_VM_LINE, vm_file)) != NULL) {
      get_asm_instructions(vm_line, asm_line);
      fputs(asm_line, asm_file);
    }

    fclose(vm_file);
    fclose(asm_file);
  }

  return (EXIT_SUCCESS);
}

static char *get_asm_filename(char *root_filename, int root_filename_len) {
  assert(strlen(root_filename) == root_filename_len);
  int asm_filename_len = root_filename_len + ASM_EXTENSION_LEN + 1;
  char *asm_filename = (char *) malloc(asm_filename_len * sizeof(char));
  if (asm_filename == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(asm_filename, root_filename);
  strncat(asm_filename, ASM_EXTENSION, ASM_EXTENSION_LEN + 1);
  return asm_filename;
}

char *get_root_filename() {
  return root_filename;
}

