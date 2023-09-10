#include <stdbool.h>

#include "AssemblyGenerator.h"

void getStackCommands(char *vmLine, char *asmLine);
void getArithmeticCommands(char *vmLine, char *asmLine);
bool isStack(char *vmLine);
bool isArithmetic(char *vmLine);

void getAsmCommands(char *vmLine, char *asmLine) {
  if (isStack(vmLine)) {
    getStackCommand(vmLine, asmLine);
  } else if (isArithmetic(vmLine)) {
    getArithmeticCommand(vmLine, asmLine);
  } else {

  }
}

void getStackCommands(char *vmLine, char *asmLine) {

}

void getArithmeticCommands(char *vmLine, char *asmLine) {

}

bool isStack(char *vmLine) {
  return !(strncmp(vmLine, "push", 4) && strncmp(vmLine, "pop", 3));
}

bool isArithmetic(char *vmLine) {
  static int numArithmeticCommands = 9;
  static char *arithmeticCommands[] = {
    "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"
  };
  for (int commandNum = 0; commandNum < numArithmeticCommands; commandNum++) {
    if (strncmp(vmLine, arithmeticCommands[commandNum],
      strlen(arithmeticCommands[commandNum])) == 0) {
        return true;
    }
  }
  return false;
}