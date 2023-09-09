#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AssemblyGenerator.h"
#include "Parser.h"

#define VM_EXTENSION_LEN  3
#define ASM_EXTENSION_LEN 4
#define VM_EXTENSION  ".vm"
#define ASM_EXTENSION ".asm"
#define MAX_LINE 40

static char *getAsmFilename(char *vmFilename);

int main(int argc, char *argv[]) {
  static char vmLine[MAX_LINE];
  static char asmLine[MAX_LINE];
  static FILE *vmFile, *asmFile;

  if (argc < 2) {  // argv[0] = VmEmulator, argc[1] = file1.vm ...
    perror("Usage: $ VmEmulator file1.vm [, file2.vm ...]");
  }
  for (int vmFileIndex = 1; vmFileIndex < argc; vmFileIndex++) {
    char *vmFilename = argv[vmFileIndex];
    if (strstr(vmFilename, VM_EXTENSION) - vmFilename !=
        strlen(vmFilename) - ASM_EXTENSION_LEN) {
      printf("Error: %s not supported, must end in %s", vmFilename, VM_EXTENSION);
      continue;
    }
    vmFile = fopen(vmFilename, "r");
    if (vmFile == NULL) {
      printf("File not found: %s\n", vmFilename);
      exit(EXIT_FAILURE);
    }

    char *asmFilename = getAsmFilename(vmFilename);
    asmFile = fopen(asmFilename, "w");
    if (asmFile == NULL) {
      printf("File not found: %s\n", asmFilename);
      exit(EXIT_FAILURE);
    }
    free(asmFilename);

    while ((getLine(vmLine, MAX_LINE, vmFile)) != NULL) {
      printf("%s\n", vmLine);
    }

    fclose(vmFile);
    fclose(asmFile);
  }

  return (EXIT_SUCCESS);
}

static char *getAsmFilename(char *vmFilename) {
  int asmFilenameLen = strlen(vmFilename) - VM_EXTENSION_LEN
      + ASM_EXTENSION_LEN + 1;
  char *asmFilename = (char *) malloc(asmFilenameLen * sizeof(char));
  if (asmFilename == NULL) {
    printf("malloc error\n");
    exit(EXIT_FAILURE);
  }
  strcpy(asmFilename, vmFilename);
  char *pExtension = strstr(asmFilename, VM_EXTENSION);
  strcpy(pExtension, ASM_EXTENSION);
  return asmFilename;
}
