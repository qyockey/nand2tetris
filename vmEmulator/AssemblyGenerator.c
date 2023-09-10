#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "AssemblyGenerator.h"

static void append(char *dest, char *src);
static void getPushInstructions(char *asmLine);
static void getPopInstructions(char *asmLine);
static void getAddInstructions(char *asmLine);
static void getSubInstructions(char *asmLine);
static void getNegInstructions(char *asmLine);
static void getEqInstructions(char *asmLine);
static void getLtInstructions(char *asmLine);
static void getGtInstructions(char *asmLine);
static void getAndInstructions(char *asmLine);
static void getOrInstructions(char *asmLine);
static void getNotInstructions(char *asmLine);
static void getBinaryOpInstructions(char *asmLine, char binaryOp);
static void getCompInstructions(char *asmLine, char *comp, int count);

static int asmLen = 0;
static int eqCount = 0;
static int ltCount = 0;
static int gtCount = 0;

static char *subInstructions = ""
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"
    "A=A-1\n"
    "M=M-D\n";
//      ^ subtraction operator to change

#define SUB_OPERATOR_INDEX -3

void getAsmInstructions(char *vmLine, char *asmLine) {

  append(asmLine, "// ");
  append(asmLine, vmLine);
  append(asmLine, "\n");
  static int numCommands = 11;
  static char *commands[] = {
    "push", "pop", "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"
  };
  static void (*getInstructionFunctions[])(char *) = {
    getPushInstructions, getPopInstructions, getAddInstructions,
    getSubInstructions, getNegInstructions, getEqInstructions,
    getGtInstructions, getLtInstructions, getAndInstructions,
    getOrInstructions, getNotInstructions
  };
  for (int cmdNum = 0; cmdNum < numCommands; cmdNum++) {
    if (strncmp(vmLine, commands[cmdNum], strlen(commands[cmdNum])) == 0) {
      getInstructionFunctions[cmdNum](asmLine);
      break;
    }
  }
  append(asmLine, "\n");
}

static void append(char *asmLine, char *data) {
  strcpy(asmLine + asmLen, data);
  asmLen += strlen(data);
}

static void getPushInstructions(char *asmLine) {
  append(asmLine, "push cmd\n");
}

static void getPopInstructions(char *asmLine) {
  append(asmLine, "pop cmd\n");
}

static void getAddInstructions(char *asmLine) {
  getBinaryOpInstructions(asmLine, '+');
}

static void getSubInstructions(char *asmLine) {
  append(asmLine, subInstructions);
}

static void getNegInstructions(char *asmLine) {
  append(asmLine, "neg cmd\n");
}

static void getEqInstructions(char *asmLine) {
  getCompInstructions(asmLine, "EQ", eqCount);
}

static void getLtInstructions(char *asmLine) {
  getCompInstructions(asmLine, "LT", ltCount);
}

static void getGtInstructions(char *asmLine) {
  getCompInstructions(asmLine, "GT", gtCount);
}

static void getAndInstructions(char *asmLine) {
  getBinaryOpInstructions(asmLine, '&');
}

static void getOrInstructions(char *asmLine) {
  getBinaryOpInstructions(asmLine, '|');
}

static void getNotInstructions(char *asmLine) {
  append(asmLine,
      "@SP\n"
      "A=M\n"
      "M=!M\n"
  );
}

static void getBinaryOpInstructions(char *asmLine, char binaryOp) {
  append(asmLine, subInstructions);
  *(asmLine + asmLen + SUB_OPERATOR_INDEX) = binaryOp;
}

static void getCompInstructions(char *asmLine, char *comp, int count) {
  append(asmLine, subInstructions);
  asmLen--; // remove \n at end
  append(asmLine, ";J");
  append(asmLine, comp);
  append(asmLine,
      "\n"
      "M=-1"
      "("
  );
  append(asmLine, comp);
  append(asmLine, "$");
  append(asmLine, itoa(count));
  append(asmLine, ".continue)\n");
}