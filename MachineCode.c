#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MachineCode.h"
#include "SymbolTable.h"

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

bool isNumber(char *label);
void removeSpaces(char *instruction);
void initCInstruction(char *instruction);
void getDestInstruction(char *dest, char *asmLine);
void getCompInstruction(char *comp, char *asmLine);
void getJumpInstruction(char *jump, char *asmLine);

void getAInstruction(char *asmLine, char *instruction) {
  char *label = strchr(asmLine, '@') + 1;
  int address;
  if (isNumber(label)) {
    address = atoi(label);
  } else if (symbolTableContains(label)) {
    address = symbolTableGet(label);
  } else {
    static int nextVarAddr = 16;
    symbolTableAdd(label, nextVarAddr);
    address = nextVarAddr++;
  }
  int i = COMMAND_LEN;
  instruction[--i] = '\0';
  while (i > 0) {
    instruction[--i] = '0' + (address & 0x01);
    address >>= 1;
  }
}

bool isNumber(char *label) {
  char c;
  while ((c = *label++) != '\0') {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

void getCInstruction(char *asmLine, char *instruction) {
  initCInstruction(instruction);
  char *dest, *comp, *jump;
  char *pSemicolon = index(asmLine, ';');
  if (pSemicolon != NULL) {
    jump = pSemicolon + 1;
    *pSemicolon = '\0';
  } else {
    jump = NULL;
  }
  char *pEquals = index(asmLine, '=');
  if (pEquals != NULL) {
    dest = asmLine;
    comp = pEquals + 1;
    *pEquals = '\0';
  } else {
    dest = NULL;
    comp = asmLine;
  }
  getDestInstruction(dest, instruction);
  getCompInstruction(comp, instruction);
  getJumpInstruction(jump, instruction);
}

void initCInstruction(char *instruction) {
  strcpy(instruction, "1110000000000000\0");
}

void getDestInstruction(char *dest, char* instruction) {
  if (dest == NULL) {
    return;
  }
  if (strchr(dest, 'A')) {
    *(instruction + DEST_A) = '1';
  }
  if (strchr(dest, 'M')) {
    *(instruction + DEST_M) = '1';
  }
  if (strchr(dest, 'D')) {
    *(instruction + DEST_D) = '1';
  }
}

void getCompInstruction(char *comp, char* instruction) {
  static char *compLookup[][LOOKUP_COLS_PER_ENTRY] = {
    { "0",   "101010"},
    { "1",   "111111"},
    { "-1",  "111010"},
    { "D",   "001100"},
    { "A",   "110000"},
    { "M",   "110000"},
    { "!D",  "001101"},
    { "!A",  "110001"},
    { "!M",  "110001"},
    { "-D",  "001111"},
    { "-A",  "110011"},
    { "-M",  "110011"},
    { "D+1", "011111"},
    { "A+1", "110111"},
    { "M+1", "110111"},
    { "D-1", "001110"},
    { "A-1", "110010"},
    { "M-1", "110010"},
    { "D+A", "000010"},
    { "D+M", "000010"},
    { "D-A", "010011"},
    { "D-M", "010011"},
    { "A-D", "000111"},
    { "M-D", "000111"},
    { "D&A", "000000"},
    { "D&M", "000000"},
    { "D|A", "010101"},
    { "D|M", "010101"}
  };

  if (strchr(comp, 'M')) {
    *(instruction + MODE) = '1';
  }
  char *compInstruction;
  int numEntries = sizeof(compLookup) / sizeof(compLookup[0]);
  for (int i = 0; i < numEntries; i ++) {
    if (strcmp(comp, compLookup[i][ASM_COL]) == 0) {
      compInstruction = compLookup[i][HACK_COL];
      break;
    }
  }
  strncpy(instruction + COMP_START, compInstruction, COMP_INSTRUCTION_LEN);
}

void getJumpInstruction(char *jump, char* instruction) {
  static char *jumpLookup[][LOOKUP_COLS_PER_ENTRY] = {
    { "",    "000"},
    { "JGT", "001"},
    { "JEQ", "010"},
    { "JGE", "011"},
    { "JLT", "100"},
    { "JNE", "101"},
    { "JLE", "110"},
    { "JMP", "111"}
  };

  char *jumpInstruction;
  if (jump == NULL) {
    jumpInstruction = "000";
  } else {
    int numEntries = sizeof(jumpLookup) / sizeof(jumpLookup[0]);
    for (int i = 0; i < numEntries; i ++) {
      if (strcmp(jump, jumpLookup[i][ASM_COL]) == 0) {
        jumpInstruction = jumpLookup[i][HACK_COL];
        break;
      }
    }
  }
  strncpy(instruction + JUMP_START, jumpInstruction, JUMP_INSTRUCTION_LEN);
}
