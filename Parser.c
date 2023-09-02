#include <stdio.h>
#include <ctype.h>

int isACommand(char *command);

int isACommand(char *command) {
  // while (isspace(*command++));
  return *command == '@';
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
