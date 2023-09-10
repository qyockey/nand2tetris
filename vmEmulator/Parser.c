#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Parser.h"

void removeComments(char *vmLine);
void removeNewlines(char *vmLine);

char *getLine(char *vmLine, int maxLine, FILE *vmFile) {
  do {
    if (fgets(vmLine, maxLine, vmFile) == NULL) {
      return NULL;
    } 
    removeComments(vmLine);
    removeNewlines(vmLine);
  } while (strlen(vmLine) == 0);
  return vmLine;
}

void removeComments(char *vmLine) {
  char *pInline = strstr(vmLine, "//");
  if (pInline) {
    *pInline = '\0';
  }
}

void removeNewlines(char *vmLine) {
  while ((*vmLine) != '\0') {
    if (*vmLine == '\r' || *vmLine == '\n') {
      *vmLine = '\0';
    }
    vmLine++;
  }
}
