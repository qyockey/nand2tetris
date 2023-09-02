#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.c"
#include "MachineCode.c"

#define ASM_EXTENSION_LEN 3
#define BIN_EXTENSION_LEN 4
#define MAX_LINE 100

static char asmLine[MAX_LINE];
static char instruction[COMMAND_LEN];
static FILE *asmFile, *binFile;

char *getBinFilename(char *asmFilename);
 
int main(int argc, char **argv) {
  if (argc != 2) {  // argv[0] = ./Main, argc[1] = asmFilename
    printf("Assembler takes 1 argument, the name of the file to assemble ending in .asm\n");
    exit(EXIT_FAILURE);
  }
  char *asmFilename = *++argv;
  char *binFilename = getBinFilename(asmFilename);

  // open files
  asmFile = fopen(asmFilename, "r");
  if (asmFile == NULL) {
    printf("File not found: %s\n", asmFilename);
    exit(EXIT_FAILURE);
  }
  binFile = fopen(binFilename, "w");
  if (binFile == NULL) {
    printf("File not found: %s\n", binFilename);
    exit(EXIT_FAILURE);
  }
  free(binFilename);

  while ((fgets(asmLine, MAX_LINE, asmFile)) != NULL) {
    removeSpaces(asmLine);
    removeComments(asmLine);
    if (strlen(asmLine) > 0) {
      if (isACommand(asmLine)) {
        getAInstruction(asmLine, instruction);
      } else {
        getCInstruction(asmLine, instruction);
      }
      fputs(instruction, binFile);
    }
  }

  fclose(asmFile);
  fclose(binFile);

  return (EXIT_SUCCESS);
}

char *getBinFilename(char *asmFilename) {
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
