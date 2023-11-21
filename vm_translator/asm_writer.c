#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm_writer.h"
#include "error_check.h"
#include "main.h"

#define LCL 1
#define ARG 2
#define THIS 3
#define THAT 4
#define TEMP_START 5
#define MAX_VM_CMD_LEN 8
#define MAX_STR_LEN 50
#define NUM_VM_CMDS 14

enum commands { 
  PUSH, POP, ADD, SUB, NEG, EQ, LT, GT, AND, OR, NOT, LABEL, GOTO, IF_GOTO
};

struct cmd_args {
  char operand[MAX_STR_LEN];
  int value;
};

static void write_vm_comment(const char *vm_line);
static void write_int(int num);
static void write_str(const char *str);
static void write_push(const struct cmd_args *args);
static void write_push_segment(const char *segment, int value);
static void write_pop(const struct cmd_args *args);
static void write_pop_segment(const char *segment, int value);
static void write_add(const struct cmd_args *args);
static void write_sub(const struct cmd_args *args);
static void write_neg(const struct cmd_args *args);
static void write_eq(const struct cmd_args *args);
static void write_lt(const struct cmd_args *args);
static void write_gt(const struct cmd_args *args);
static void write_and(const struct cmd_args *args);
static void write_or(const struct cmd_args *args);
static void write_not(const struct cmd_args *args);
static void write_binary_op(const char *binary_op);
static void write_comp(const char *comp, int count);
static void write_label(const struct cmd_args *args);
static void write_goto(const struct cmd_args *args);
static void write_if_goto(const struct cmd_args *args);

struct vm_command {
  enum commands operator;
  char *name;
  void (*write_function)(const struct cmd_args *args);
};
struct vm_command cmd_list[] = {
  { PUSH,    "push",    write_push    },
  { POP,     "pop",     write_pop     },
  { ADD,     "add",     write_add     },
  { SUB,     "sub",     write_sub     },
  { NEG,     "neg",     write_neg     },
  { EQ,      "eq",      write_eq      },
  { LT,      "lt",      write_lt      },
  { GT,      "gt",      write_gt      },
  { AND,     "and",     write_and     },
  { OR,      "or",      write_or      },
  { NOT,     "not",     write_not     },
  { LABEL,   "label",   write_label   },
  { GOTO,    "goto",    write_goto    },
  { IF_GOTO, "if-goto", write_if_goto }
};

static char *push_instructions = ""
    "@SP\n"
    "AM=M+1\n"
    "A=A-1\n"
    "M=D\n";

static FILE *asm_file;
static char *root_file_name;

void writer_init(FILE *out_file, char *out_file_root_name) {
  asm_file = out_file;
  root_file_name = out_file_root_name;
}

static void write_vm_comment(const char *vm_line) {
  write_str("// ");
  write_str(vm_line);
  write_str("\n");
}

static struct vm_command *get_command(const char *operator) {
  for (int cmd_index = 0; cmd_index < NUM_VM_CMDS; cmd_index++) {
    if (strcmp(operator, cmd_list[cmd_index].name) == EXIT_SUCCESS) {
      return cmd_list + cmd_index;
    }
  }
  fprintf(stderr, "Error: invalid operator \"%s\"\n", operator);
  exit(EXIT_FAILURE);
}

void write_asm_instructions(const char *vm_line) {
  write_vm_comment(vm_line);
  char operator[MAX_STR_LEN];
  struct cmd_args *args = (struct cmd_args *) malloc(sizeof(struct cmd_args *));
  sscanf(vm_line, "%s %s %d", operator, args->operand, &args->value);
  const struct vm_command *cmd = get_command(operator);
  cmd->write_function(args);
  write_str("\n");
}

static void write_str(const char *str) {
  fprintf(asm_file, "%s", str);
}

static void write_int(int num) {
  fprintf(asm_file, "%d", num);
}

static void write_push(const struct cmd_args *args) {
  if (strcmp(args->operand, "local") == EXIT_SUCCESS) {
    // push RAM[*segment_pointer + i]
    write_push_segment("LCL", args->value);
  } else if (strcmp(args->operand, "argument") == EXIT_SUCCESS) {
    // push RAM[*segment_pointer + i]
    write_push_segment("ARG", args->value);
  } else if (strcmp(args->operand, "this") == EXIT_SUCCESS) {
    // push RAM[*segment_pointer + i]
    write_push_segment("THIS", args->value);
  } else if (strcmp(args->operand, "that") == EXIT_SUCCESS) {
    // push RAM[*segment_pointer + i]
    write_push_segment("THAT", args->value);
  } else if (strcmp(args->operand, "constant") == EXIT_SUCCESS) {
    // push i
    write_str("@");
    write_int(args->value);
    write_str("\n");
    write_str("D=A\n");
    write_str(push_instructions);
  } else if (strcmp(args->operand, "static") == EXIT_SUCCESS) {
    // push variable foo.i
    write_str("@");
    write_str(root_file_name);
    write_str(".");
    write_int(args->value);
    write_str("\n");
    write_str("D=M\n");
    write_str(push_instructions);
  } else if (strcmp(args->operand, "temp") == EXIT_SUCCESS) {
    // push RAM[*(5+i)]
    write_str("@R");
    write_int(TEMP_START + args->value);
    write_str("\n");
    write_str("D=M\n");
    write_str(push_instructions);
  } else if (strcmp(args->operand, "pointer") == EXIT_SUCCESS) {
    // 0 => push this
    // 1 => push that
    write_str("@");
    if (args->value == 0) {
      write_str("THIS");
    } else if (args->value == 1) {
      write_str("THAT");
    }
    write_str(""
        "\n"
        "D=M\n"
    );
    write_str(push_instructions);
  }
}

static void write_push_segment(const char *segment, int value) {
  write_str("@");
  write_str(segment);
  write_str(""
      "\n"
      "D=M\n"
      "@"
  );
  write_int(value);
  write_str(""
      "\n"
      "A=D+A\n"
      "D=M\n"
  );
  write_str(push_instructions);
}

static void write_pop(const struct cmd_args *args) {
  if (strcmp(args->operand, "local") == EXIT_SUCCESS) {
    // pop RAM[*segment_pointer + i]
    write_pop_segment("LCL", args->value);
  } else if (strcmp(args->operand, "argument") == EXIT_SUCCESS) {
    // pop RAM[*segment_pointer + i]
    write_pop_segment("ARG", args->value);
  } else if (strcmp(args->operand, "this") == EXIT_SUCCESS) {
    // pop RAM[*segment_pointer + i]
    write_pop_segment("THIS", args->value);
  } else if (strcmp(args->operand, "that") == EXIT_SUCCESS) {
    // pop RAM[*segment_pointer + i]
    write_pop_segment("THAT", args->value);
  } else if (strcmp(args->operand, "static") == EXIT_SUCCESS) {
    // pop variable foo.i
    write_str(""
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@"
    );
    write_str(root_file_name);
    write_str(".");
    write_int(args->value);
    write_str(""
        "\n"
        "M=D\n"
    );
  } else if (strcmp(args->operand, "temp") == EXIT_SUCCESS) {
    // pop RAM[*(5+i)]
    write_str(""
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@R"
    );
    write_int(TEMP_START + args->value);
    write_str(""
        "\n"
        "M=D\n"
    );
  } else if (strcmp(args->operand, "pointer") == EXIT_SUCCESS) {
    // 0 => pop this
    // 1 => pop that
    write_str(""
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@"
    );
    if (args->value == 0) {
      write_str("THIS");
    } else if (args->value == 1) {
      write_str("THAT");
    }
    write_str(""
        "\n"
        "M=D\n"
    );
  }
}

static void write_pop_segment(const char *segment, int value) {
  write_str("@");
  write_str(segment);
  write_str(""
      "\n"
      "D=M\n"
      "@"
  );
  write_int(value);
  write_str(""
      "\n"
      "D=A+D\n"
      "@R13\n"
      "M=D\n"
  );
  write_str(""
      "@SP\n"
      "AM=M-1\n"
      "D=M\n"
      "@R13\n"
      "A=M\n"
      "M=D\n"
  );
}

static void write_add(__attribute__((unused)) const struct cmd_args *args) {
  write_binary_op("+");
}

static void write_sub(__attribute__((unused)) const struct cmd_args *args) {
  write_binary_op("-");
}

static void write_neg(__attribute__((unused)) const struct cmd_args *args) {
  write_str(""
      "@SP\n"
      "A=M-1\n"
      "M=-M\n"
  );
}

static void write_eq(__attribute__((unused)) const struct cmd_args *args) {
  static int eq_count = 0;
  write_comp("JEQ", eq_count);
  eq_count++;
}

static void write_lt(__attribute__((unused)) const struct cmd_args *args) {
  static int lt_count = 0;
  write_comp("JEQ", lt_count);
  lt_count++;
}

static void write_gt(__attribute__((unused)) const struct cmd_args *args) {
  static int gt_count = 0;
  write_comp("JEQ", gt_count);
  gt_count++;
}

static void write_and(__attribute__((unused)) const struct cmd_args *args) {
  write_binary_op("&");
}

static void write_or(__attribute__((unused)) const struct cmd_args *args) {
  write_binary_op("|");
}

static void write_not(__attribute__((unused)) const struct cmd_args *args) {
  write_str(""
      "@SP\n"
      "A=M-1\n"
      "M=!M\n"
  );
}

static void write_binary_op(const char *binary_op) {
  write_str(""
      "@SP\n"
      "AM=M-1\n"
      "D=M\n"
      "A=A-1\n"
      "M=M"
  );
  write_str(binary_op);
  write_str("D\n");
}

static void write_comp(const char *comp, int count) {
  char *label = (char *) malloc(MAX_STR_LEN * sizeof(char));
  assert_malloc_success((void *) label);
  sprintf(label, "%s%d", comp, count);

  write_str(""
      "@SP\n"
      "AM=M-1\n"
      "D=M\n"
      "A=A-1\n"
      "D=M-D\n"
      "M=-1\n"
      "@"
  );
  write_str(label);
  write_str(""
      "\n"
      "D;"
  );
  write_str(comp);
  write_str(""
      "\n"
      "@SP\n"
      "A=M-1\n"
      "M=0\n"
  );
  write_str("(");
  write_str(label);
  write_str(")\n");
  free(label);
}

static void write_label(const struct cmd_args *args) {
  write_str("(");
  write_str(args->operand);
  write_str(")\n");
}

static void write_goto(const struct cmd_args *args) {
  write_str("@");
  write_str(args->operand);
  write_str(""
      "\n"
      "0;JMP\n"
  );
}

static void write_if_goto(const struct cmd_args *args) {
  write_str(""
      "@SP\n"
      "AM=M-1\n"
      "D=A+1\n"
  );
  write_str("@");
  write_str(args->operand);
  write_str(""
      "\n"
      "D;JNE\n"
  );

}

