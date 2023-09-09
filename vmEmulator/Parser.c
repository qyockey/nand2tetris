#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Parser.h"

void removeComments(char *vmLine);

char *getLine(char *vmLine, int maxLine, FILE *vmFile) {
  do {
    if (fgets(vmLine, maxLine, vmFile) == NULL) {
      return NULL;
    } 
    removeComments(vmLine);
  } while (strlen(vmLine) == 0);
  return vmLine;
}

void removeComments(char *vmLine) {
  char *pInline = strstr(vmLine, "//");
  if (pInline) {
    *pInline = '\0';
  }
}

bool isStack(char *vmLine) {

}

bool isArithmetic(char *vmLine) {

}
