#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "Parser.h"

char *getLine(char *asmLine, int maxLine, FILE *asmFile) {
  asmLine = fgets(asmLine, maxLine, asmFile);
  if (asmLine == NULL) {
    return asmLine;
  } 
  removeComments(asmLine);
  removeSpaces(asmLine);
  if (strlen(asmLine) == 0) {
    getLine(asmLine, maxLine, asmFile);
  }
  return asmLine;
}

bool isLabel(char *asmLine) {
  return *asmLine == '(';
}

char *getLabel(char *asmLine) {
  asmLine++;
  char *rightParenIndex = strchr(asmLine, ')');
  *rightParenIndex = '\0';
  return asmLine;
}

bool isACommand(char *asmLine) {
  return *asmLine == '@';
}

void removeSpaces(char *asmLine) {
  char *noSpaces = asmLine;
  char c;
  while ((c = *asmLine) != '\0') {
    if (!isspace(c)) {
      *noSpaces++ = c;
    }
    asmLine++;
  }
  *noSpaces = '\0';
}

void removeComments(char *asmLine) {
  char *pInline = strstr(asmLine, "//");
  if (pInline) {
    *pInline = '\0';
  }
}
