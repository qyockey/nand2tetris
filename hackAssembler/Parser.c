#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Parser.h"

void removeSpaces(char *asmLine);
void removeComments(char *asmLine);

char *getLine(char *asmLine, int maxLine, FILE *asmFile) {
  do {
    if (fgets(asmLine, maxLine, asmFile) == NULL) {
      return NULL;
    } 
    removeComments(asmLine);
    removeSpaces(asmLine);
  } while (strlen(asmLine) == 0);
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
