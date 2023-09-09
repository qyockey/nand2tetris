#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"
#include "MachineCode.h"
#include "SymbolTable.h"

#define ASM_EXTENSION_LEN 4
#define BIN_EXTENSION_LEN 5
#define MAX_LINE 100

static char *getBinFilename(char *asmFilename);

int main(int argc, char *argv[]) {
  static char asmLine[MAX_LINE];
  static char instruction[COMMAND_LEN];
  static FILE *asmFile, *binFile;

  symbolTableInit();

  if (argc < 2) {  // argv[0] = ./Main, argc[1] = asmFilename ...
    printf("Assembler takes at least one argument, the names of files to assemble ending in .asm\n");
    exit(EXIT_FAILURE);
  }
  for (int asmFileIndex = 1; asmFileIndex < argc; asmFileIndex++) {
    char *asmFilename = argv[asmFileIndex];
    if (strstr(asmFilename, ".asm") - asmFilename != strlen(asmFilename) - ASM_EXTENSION_LEN) {
      printf("Error: file to assemble must end in .asm\n");
    }
    asmFile = fopen(asmFilename, "r");
    if (asmFile == NULL) {
      printf("File not found: %s\n", asmFilename);
      exit(EXIT_FAILURE);
    }

    int lineNum = 0;
    while ((getLine(asmLine, MAX_LINE, asmFile)) != NULL) {
      if (isLabel(asmLine)) {
        symbolTableAdd(getLabel(asmLine), lineNum);
      } else {
        lineNum++;
      }
    }

    char *binFilename = getBinFilename(asmFilename);
    binFile = fopen(binFilename, "w");
    if (binFile == NULL) {
      printf("File not found: %s\n", binFilename);
      exit(EXIT_FAILURE);
    }
    free(binFilename);

    rewind(asmFile);
    while ((getLine(asmLine, MAX_LINE, asmFile)) != NULL) {
      if (isACommand(asmLine)) {
        getAInstruction(asmLine, instruction);
      } else if (isLabel(asmLine)) {
        continue;
      } else {
        getCInstruction(asmLine, instruction);
      }
      fprintf(binFile, "%s\n", instruction);
    }

    fclose(asmFile);
    fclose(binFile);
  }

  return (EXIT_SUCCESS);
}

static char *getBinFilename(char *asmFilename) {
  int binFilenameLen = strlen(asmFilename) - ASM_EXTENSION_LEN
      + BIN_EXTENSION_LEN + 1;
  char *binFilename = (char *) malloc(binFilenameLen * sizeof(char));
  if (binFilename == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(binFilename, asmFilename);
  char *pExtension = strstr(binFilename, ".asm");
  strcpy(pExtension, ".hack");
  return binFilename;
}
