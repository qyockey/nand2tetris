#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm_writer.h"
#include "error_check.h"
#include "hash_table.h"

#define LCL 1
#define ARG 2
#define THIS 3
#define THAT 4
#define TEMP_START 5
#define MAX_VM_CMD_LEN 8
#define MAX_STR_LEN 50
#define NUM_VM_CMDS 17
#define CALL_COUNT_WIDTH 4
#define CALL_COUNT_TABLE_SIZE 100

#define PUSH_D \
        "@SP\n" \
        "AM=M+1\n" \
        "A=A-1\n" \
        "M=D\n" \

#define PUSH_A \
        "D=A\n" \
        PUSH_D

#define PUSH_M \
        "D=M\n" \
        PUSH_D

#define POP_D \
        "@SP\n" \
        "AM=M-1\n" \
        "D=M\n"

enum commands { 
    PUSH, POP, ADD, SUB, NEG, EQ, LT, GT, AND, OR, NOT, LABEL, GOTO, IF_GOTO,
    CALL, FUNCTION, RETURN
};

typedef struct {
    char operand[MAX_STR_LEN];
    unsigned value;
    char vm_file_name[MAX_STR_LEN];
} cmd_args;

typedef struct {
    enum commands operator;
    char *name;
    void (*write_function)(const cmd_args *args);
} vm_command;

static void write_vm_comment(const char *vm_line);
static void write_push(const cmd_args *args);
static void write_push_segment(const char *segment, int value);
static void write_pop(const cmd_args *args);
static void write_pop_segment(const char *segment, int value);
static void write_add(const cmd_args *args);
static void write_sub(const cmd_args *args);
static void write_neg(const cmd_args *args);
static void write_eq(const cmd_args *args);
static void write_lt(const cmd_args *args);
static void write_gt(const cmd_args *args);
static void write_and(const cmd_args *args);
static void write_or(const cmd_args *args);
static void write_not(const cmd_args *args);
static void write_label(const cmd_args *args);
static void write_goto(const cmd_args *args);
static void write_if_goto(const cmd_args *args);
static void write_call(const cmd_args *args);
static void write_function(const cmd_args *args);
static void write_return(const cmd_args *args);
static void write_binary_op(const char binary_op);
static void write_comp(const char *comp, int count);
static char *get_return_label(const char *function_name);

static vm_command cmd_list[] = {
    { PUSH,     "push",     write_push     },
    { POP,      "pop",      write_pop      },
    { ADD,      "add",      write_add      },
    { SUB,      "sub",      write_sub      },
    { NEG,      "neg",      write_neg      },
    { EQ,       "eq",       write_eq       },
    { LT,       "lt",       write_lt       },
    { GT,       "gt",       write_gt       },
    { AND,      "and",      write_and      },
    { OR,       "or",       write_or       },
    { NOT,      "not",      write_not      },
    { LABEL,    "label",    write_label    },
    { GOTO,     "goto",     write_goto     },
    { IF_GOTO,  "if-goto",  write_if_goto  },
    { CALL,     "call",     write_call     },
    { FUNCTION, "function", write_function },
    { RETURN,   "return",   write_return   }
};

static FILE *asm_file;
static hash_table *call_counts;

void writer_init(const char *asm_file_path) {
    asm_file = safe_fopen(asm_file_path, "w");
    call_counts = hash_table_init(CALL_COUNT_TABLE_SIZE);
}

void writer_dispose() {
    safe_fclose(asm_file);
    hash_table_dispose(call_counts);
}

void write_bootstrap() {
    fprintf(asm_file, 
            "// bootstrap code\n"
            "@256\n"
            "D=A\n"
            "@SP\n"
            "M=D\n"
            "\n"
    );
    write_asm_instructions("call Sys.init 0", "Sys");
}

static void write_vm_comment(const char *vm_line) {
    fprintf(asm_file, "// %s\n", vm_line);
}

static vm_command *get_command(const char *operator) {
    for (int cmd_index = 0; cmd_index < NUM_VM_CMDS; cmd_index++) {
        if (strcmp(operator, cmd_list[cmd_index].name) == EXIT_SUCCESS) {
            return cmd_list + cmd_index;
        }
    }
    fprintf(stderr, "Error: invalid operator \"%s\"\n", operator);
    exit(EXIT_FAILURE);
}

void write_asm_instructions(const char *vm_line, const char *vm_file_name) {
    write_vm_comment(vm_line);
    char operator[MAX_STR_LEN];
    cmd_args args;
    sscanf(vm_line, "%s %s %u", operator, args.operand, &(args.value));
    strncpy(args.vm_file_name, vm_file_name, MAX_STR_LEN);
    const vm_command *cmd = get_command(operator);
    cmd->write_function(&args);
    fprintf(asm_file, "\n");
}

static void write_push(const cmd_args *args) {
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
        fprintf(asm_file,
                "@%d\n"
                PUSH_A,
                args->value
        );
    } else if (strcmp(args->operand, "static") == EXIT_SUCCESS) {
        // push variable foo.i
        fprintf(asm_file,
                "@%s.%d\n"
                PUSH_M,
                args->vm_file_name, args->value
        );
    } else if (strcmp(args->operand, "temp") == EXIT_SUCCESS) {
        // push RAM[*(5+i)]
        fprintf(asm_file,
                "@R%d\n"
                PUSH_M,
                TEMP_START + args->value
        );
    } else if (strcmp(args->operand, "pointer") == EXIT_SUCCESS) {
        // 0 => push this
        // 1 => push that
        fprintf(asm_file, "@");
        if (args->value == 0) {
            fprintf(asm_file, "THIS");
        } else if (args->value == 1) {
            fprintf(asm_file, "THAT");
        }
        fprintf(asm_file,
                "\n"
                PUSH_M
        );
    }
}

static void write_push_segment(const char *segment, int value) {
    fprintf(asm_file,
            "@%s\n"
            "D=M\n"
            "@%d\n"
            "A=D+A\n"
            PUSH_M,
            segment, value
    );
}

static void write_pop(const cmd_args *args) {
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
        fprintf(asm_file,
                POP_D
                "@%s.%d\n"
                "M=D\n",
                args->vm_file_name, args->value
        );
    } else if (strcmp(args->operand, "temp") == EXIT_SUCCESS) {
        // pop RAM[*(5+i)]
        fprintf(asm_file,
                POP_D
                "@R%d\n"
                "M=D\n",
                args->value + TEMP_START
        );
    } else if (strcmp(args->operand, "pointer") == EXIT_SUCCESS) {
        // 0 => pop this
        // 1 => pop that
        fprintf(asm_file,
                POP_D
                "@"
        );
        if (args->value == 0) {
            fprintf(asm_file, "THIS");
        } else if (args->value == 1) {
            fprintf(asm_file, "THAT");
        }
        fprintf(asm_file,
                "\n"
                "M=D\n"
        );
    }
}

static void write_pop_segment(const char *segment, int value) {
    fprintf(asm_file,
            "@%s\n"
            "D=M\n"
            "@%d\n"
            "D=A+D\n"
            "@R13\n"
            "M=D\n"
            POP_D
            "@R13\n"
            "A=M\n"
            "M=D\n",
            segment, value
    );
}

static void write_add(__attribute__((unused)) const cmd_args *args) {
    write_binary_op('+');
}

static void write_sub(__attribute__((unused)) const cmd_args *args) {
    write_binary_op('-');
}

static void write_neg(__attribute__((unused)) const cmd_args *args) {
    fprintf(asm_file,
            "@SP\n"
            "A=M-1\n"
            "M=-M\n"
    );
}

static void write_eq(__attribute__((unused)) const cmd_args *args) {
    static int eq_count = 0;
    write_comp("JEQ", eq_count);
    eq_count++;
}

static void write_lt(__attribute__((unused)) const cmd_args *args) {
    static int lt_count = 0;
    write_comp("JLT", lt_count);
    lt_count++;
}

static void write_gt(__attribute__((unused)) const cmd_args *args) {
    static int gt_count = 0;
    write_comp("JGT", gt_count);
    gt_count++;
}

static void write_and(__attribute__((unused)) const cmd_args *args) {
    write_binary_op('&');
}

static void write_or(__attribute__((unused)) const cmd_args *args) {
    write_binary_op('|');
}

static void write_not(__attribute__((unused)) const cmd_args *args) {
    fprintf(asm_file,
            "@SP\n"
            "A=M-1\n"
            "M=!M\n"
    );
}

static void write_label(const cmd_args *args) {
    fprintf(asm_file, "(%s)\n", args->operand);
}

static void write_goto(const cmd_args *args) {
    fprintf(asm_file,
            "@%s\n"
            "0;JMP\n",
            args->operand
    );
}

static void write_if_goto(const cmd_args *args) {
    fprintf(asm_file,
            POP_D
            "@%s\n"
            "D;JNE\n",
            args->operand
    );
}

static void write_call(const cmd_args *args) {
    const char *function_name = args->operand;
    int num_args = args->value;
    char *return_label = get_return_label(function_name);
    fprintf(asm_file,
            // push return_label
            "@%s\n"
            PUSH_A
            // push LCL
            "@LCL\n"
            PUSH_M
            // push ARG
            "@ARG\n"
            PUSH_M
            // push THIS
            "@THIS\n"
            PUSH_M
            // push THAT
            "@THAT\n"
            PUSH_M
            // LCL = SP
            "@SP\n"
            "D=M\n"
            "@LCL\n"
            "M=D\n"
            // ARG = SP - 5 - num_args
            "@%d\n"
            "D=D-A\n"
            "@ARG\n"
            "M=D\n"
            // goto function_name
            "@%s\n"
            "0;JMP\n"
            // return label
            "(%s)\n",
            return_label, 5 + num_args, function_name, return_label
    );
    free(return_label);
}

static void write_function(const cmd_args *args) {
    static const cmd_args constant_zero = {"constant", 0};
    const char *function_name = args->operand;
    int num_vars = args->value;
    fprintf(asm_file, "(%s)\n", function_name);
    for (int i = 0; i < num_vars; i++) {
        write_push(&constant_zero);
    }
}

static void write_return(__attribute__((unused)) const cmd_args *args) {
    fprintf(asm_file,
            // end_frame = LCL
            "@LCL\n"
            "D=M\n"
            "@R13\n"
            "M=D\n"
            // return_label = *(endframe - 5)
            "@5\n"
            "A=D-A\n"
            "D=M\n"
            "@R14\n"
            "M=D\n"
            // *ARG = pop() <- return value into ARG[0]
            POP_D
            "@ARG\n"
            "A=M\n"
            "M=D\n"
            // SP = ARG + 1
            "@ARG\n"
            "D=M\n"
            "@SP\n"
            "M=D+1\n"
            // THAT = *(end_frame - 1)
            "@R13\n"
            "A=M-1\n"
            "D=M\n"
            "@THAT\n"
            "M=D\n"
            // THIS = *(end_frame - 2)
            "@R13\n"
            "A=M-1\n"
            "A=A-1\n"
            "D=M\n"
            "@THIS\n"
            "M=D\n"
            // ARG = *(end_frame - 3)
            "D=A\n"
            "@R13\n"
            "A=M-D\n"
            "D=M\n"
            "@ARG\n"
            "M=D\n"
            // LCL = *(end_frame - 4)
            "@4\n"
            "D=A\n"
            "@R13\n"
            "A=M-D\n"
            "D=M\n"
            "@LCL\n"
            "M=D\n"
            // goto return_label
            "@R14\n"
            "A=M\n"
            "0;JMP\n"
    );
}

static void write_binary_op(const char binary_op) {
    fprintf(asm_file,
            POP_D
            "A=A-1\n"
            "M=M%cD\n",
            binary_op
    );
}

static void write_comp(const char *comp, int count) {
    char *label = safe_malloc(MAX_STR_LEN * sizeof(char));
    sprintf(label, "%s%d", comp, count);

    fprintf(asm_file,
            POP_D
            "A=A-1\n"
            "D=M-D\n"
            "M=-1\n"
            "@%s\n"
            "D;%s\n"
            "@SP\n"
            "A=M-1\n"
            "M=0\n"
            "(%s)\n",
            label, comp, label
    );
    free(label);
}

static char *get_return_label(const char *function_name) {
    int call_count;
    if (hash_table_contains(call_counts, function_name)) {
        call_count = hash_table_get(call_counts, function_name);
        call_count++;
        hash_table_set(call_counts, function_name, call_count);
    } else {
        call_count = 0;
        hash_table_add(call_counts, function_name, call_count);
    }
    size_t return_label_len = (strlen(function_name) + 5 /* strlen("$ret.") */
            + CALL_COUNT_WIDTH) * sizeof(char);
    char *return_label = safe_malloc(return_label_len);
    sprintf(return_label, "%s$ret.%0*d", function_name, CALL_COUNT_WIDTH,
            call_count);
    return return_label;
}

