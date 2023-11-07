#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "machine_code.h"
#include "symbol_table.h"

#define ASM_EXTENSION_LEN 4
#define BIN_EXTENSION_LEN 5
#define MAX_LINE 100

static char *get_bin_filename(char *asm_filename);

int main(int argc, char *argv[]) {
  static char asm_line[MAX_LINE];
  static char instruction[COMMAND_LEN];
  static FILE *asm_file, *bin_file;

  symbol_table_init();

  if (argc < 2) {  // argv[0] = ./hack_assembler, argc[1] = asm_filename ...
    printf("assembler takes at least one argument, the names of files to assemble ending in .asm\n");
    exit(EXIT_FAILURE);
  }
  for (int asm_file_index = 1; asm_file_index < argc; asm_file_index++) {
    char *asm_filename = argv[asm_file_index];
    if (strstr(asm_filename, ".asm") - asm_filename != strlen(asm_filename) - ASM_EXTENSION_LEN) {
      printf("error: file to assemble must end in .asm\n");
    }
    asm_file = fopen(asm_filename, "r");
    if (asm_file == NULL) {
      printf("file not found: %s\n", asm_filename);
      exit(EXIT_FAILURE);
    }

    int line_num = 0;
    while ((get_line(asm_line, MAX_LINE, asm_file)) != NULL) {
      if (is_label(asm_line)) {
        symbol_table_add(get_label(asm_line), line_num);
      } else {
        line_num++;
      }
    }

    char *bin_filename = get_bin_filename(asm_filename);
    bin_file = fopen(bin_filename, "w");
    if (bin_file == NULL) {
      printf("file not found: %s\n", bin_filename);
      exit(EXIT_FAILURE);
    }
    free(bin_filename);

    rewind(asm_file);
    while ((get_line(asm_line, MAX_LINE, asm_file)) != NULL) {
      if (is_a_command(asm_line)) {
        get_a_instruction(asm_line, instruction);
      } else if (is_label(asm_line)) {
        continue;
      } else {
        get_c_instruction(asm_line, instruction);
      }
      fprintf(bin_file, "%s\n", instruction);
    }

    fclose(asm_file);
    fclose(bin_file);
  }

  return (EXIT_SUCCESS);
}

static char *get_bin_filename(char *asm_filename) {
  int bin_filename_len = strlen(asm_filename) - ASM_EXTENSION_LEN
      + BIN_EXTENSION_LEN + 1;
  char *bin_filename = (char *) malloc(bin_filename_len * sizeof(char));
  if (bin_filename == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(bin_filename, asm_filename);
  char *p_extension = strstr(bin_filename, ".asm");
  strcpy(p_extension, ".hack");
  return bin_filename;
}

