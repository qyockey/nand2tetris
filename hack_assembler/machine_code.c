#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine_code.h"
#include "symbol_table.h"

#define MODE                   3
#define COMP_START             4
#define COMP_INSTRUCTION_LEN   6
#define DEST_A                10
#define DEST_D                11
#define DEST_M                12
#define JUMP_START            13
#define JUMP_INSTRUCTION_LEN   3
#define LOOKUP_COLS_PER_ENTRY  2
#define ASM_COL                0
#define HACK_COL               1
#define STACK_START           16

bool is_number(char *label);
void remove_spaces(char *instruction);
void init_c_instruction(char *instruction);
void get_dest_instruction(char *dest, char *asm_line);
void get_comp_instruction(char *comp, char *asm_line);
void get_jump_instruction(char *jump, char *asm_line);

void get_a_instruction(char *asm_line, char *instruction) {
  char *label = strchr(asm_line, '@') + 1;
  int address;
  if (is_number(label)) {
    address = atoi(label);
  } else if (symbol_table_contains(label)) {
    address = symbol_table_get(label);
  } else {
    static int next_var_addr = STACK_START;
    symbol_table_add(label, next_var_addr);
    address = next_var_addr++;
  }
  int i = COMMAND_LEN;
  instruction[--i] = '\0';
  while (i > 0) {
    instruction[--i] = '0' + (address & 0x01);
    address >>= 1;
  }
}

bool is_number(char *label) {
  char c;
  while ((c = *label++) != '\0') {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

void get_c_instruction(char *asm_line, char *instruction) {
  init_c_instruction(instruction);
  char *dest, *comp, *jump;
  char *p_semicolon = index(asm_line, ';');
  if (p_semicolon != NULL) {
    jump = p_semicolon + 1;
    *p_semicolon = '\0';
  } else {
    jump = NULL;
  }
  char *p_equals = index(asm_line, '=');
  if (p_equals != NULL) {
    dest = asm_line;
    comp = p_equals + 1;
    *p_equals = '\0';
  } else {
    dest = NULL;
    comp = asm_line;
  }
  get_dest_instruction(dest, instruction);
  get_comp_instruction(comp, instruction);
  get_jump_instruction(jump, instruction);
}

void init_c_instruction(char *instruction) {
  strcpy(instruction, "1110000000000000\0");
}

void get_dest_instruction(char *dest, char* instruction) {
  if (dest == NULL) {
    return;
  }
  if (strchr(dest, 'a')) {
    *(instruction + DEST_A) = '1';
  }
  if (strchr(dest, 'm')) {
    *(instruction + DEST_M) = '1';
  }
  if (strchr(dest, 'd')) {
    *(instruction + DEST_D) = '1';
  }
}

void get_comp_instruction(char *comp, char* instruction) {
  static char *comp_lookup[][LOOKUP_COLS_PER_ENTRY] = {
    { "0",   "101010"},
    { "1",   "111111"},
    { "-1",  "111010"},
    { "d",   "001100"},
    { "a",   "110000"},
    { "m",   "110000"},
    { "!d",  "001101"},
    { "!a",  "110001"},
    { "!m",  "110001"},
    { "-d",  "001111"},
    { "-a",  "110011"},
    { "-m",  "110011"},
    { "d+1", "011111"},
    { "a+1", "110111"},
    { "m+1", "110111"},
    { "d-1", "001110"},
    { "a-1", "110010"},
    { "m-1", "110010"},
    { "d+a", "000010"},
    { "d+m", "000010"},
    { "d-a", "010011"},
    { "d-m", "010011"},
    { "a-d", "000111"},
    { "m-d", "000111"},
    { "d&a", "000000"},
    { "d&m", "000000"},
    { "d|a", "010101"},
    { "d|m", "010101"}
  };

  if (strchr(comp, 'm')) {
    *(instruction + MODE) = '1';
  }
  char *comp_instruction;
  int num_entries = sizeof(comp_lookup) / sizeof(comp_lookup[0]);
  for (int i = 0; i < num_entries; i ++) {
    if (strcmp(comp, comp_lookup[i][ASM_COL]) == 0) {
      comp_instruction = comp_lookup[i][HACK_COL];
      break;
    }
  }
  strncpy(instruction + COMP_START, comp_instruction, COMP_INSTRUCTION_LEN);
}

void get_jump_instruction(char *jump, char* instruction) {
  static char *jump_lookup[][LOOKUP_COLS_PER_ENTRY] = {
    { "",    "000"},
    { "j_gT", "001"},
    { "j_eQ", "010"},
    { "j_gE", "011"},
    { "j_lT", "100"},
    { "j_nE", "101"},
    { "j_lE", "110"},
    { "j_mP", "111"}
  };

  char *jump_instruction;
  if (jump == NULL) {
    jump_instruction = "000";
  } else {
    int num_entries = sizeof(jump_lookup) / sizeof(jump_lookup[0]);
    for (int i = 0; i < num_entries; i ++) {
      if (strcmp(jump, jump_lookup[i][ASM_COL]) == 0) {
        jump_instruction = jump_lookup[i][HACK_COL];
        break;
      }
    }
  }
  strncpy(instruction + JUMP_START, jump_instruction, JUMP_INSTRUCTION_LEN);
}

