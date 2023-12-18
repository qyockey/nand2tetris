#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm_writer.h"
#include "error_check.h"
#include "hash_table.h"

#define TEMP_START 5
#define MAX_OPERATOR_LEN 9
#define MAX_OPERAND_LEN 50
#define NUM_VM_OPERATORS 17
#define VM_FILE_NAME_MAX_LEN 50
#define CALL_COUNT_LEN 4
#define CALL_COUNT_TABLE_SIZE 100

#define PUSH_D \
        "@SP\n" \
        "AM=M+1\n" \
        "A=A-1\n" \
        "M=D\n" 

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

typedef struct {
    char operand[MAX_OPERAND_LEN];
    unsigned value;
} cmd_args;

typedef struct {
    char *name;
    void (*write_function)(const cmd_args *args);
} vm_command;

static void write_vm_comment(const char *vm_line);
static vm_command *get_vm_command(const char *operator);
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
static void write_binary_operation(const char binary_operator);
static void write_comparison(const char *jump_code, int count);
static char *get_return_label(const char *function_name);

static vm_command cmd_list[] = {
    { "push",     write_push     },
    { "pop",      write_pop      },
    { "add",      write_add      },
    { "sub",      write_sub      },
    { "neg",      write_neg      },
    { "eq",       write_eq       },
    { "lt",       write_lt       },
    { "gt",       write_gt       },
    { "and",      write_and      },
    { "or",       write_or       },
    { "not",      write_not      },
    { "label",    write_label    },
    { "goto",     write_goto     },
    { "if-goto",  write_if_goto  },
    { "call",     write_call     },
    { "function", write_function },
    { "return",   write_return   }
};

static FILE *asm_file;
static char current_vm_file_name[VM_FILE_NAME_MAX_LEN];
static hash_table *call_counts;

/*******************************************************************************
** Function: writer_init
** Description: Initializes writer to start writing assembly instructions. Opens
**     assembly file for writing and initializes a hash table to keep track of
**     how many times functions are called.
** Parameters:
**     - asm_file_path: path of assembly file to write to
** Pre-Conditions: asm_file_path is non-null
** Post-Conditions: asm_file and call_counts are non-null
*******************************************************************************/
void writer_init(const char *asm_file_path) {
    asm_file = safe_fopen(asm_file_path, "w");
    call_counts = hash_table_init(CALL_COUNT_TABLE_SIZE);
}

/*******************************************************************************
** Function: writer_dispose
** Description: Frees all memory allocated to writer: closes assembly file
**     and disposes of call count hash table
** Parameters: void
** Pre-Conditions: current_vm_file_name is allocated (set_current_vm_file_name
**     has been called)
** Post-Conditions: All memory allocated to writer is freed
*******************************************************************************/
void writer_dispose() {
    safe_fclose(asm_file);
    hash_table_dispose(call_counts);
}

/*******************************************************************************
** Function: write_bootstrap
** Description: Writes assembly code to set stack pointer to 256, and then call
**     the Sys.init subroutine
** Parameters: void
** Pre-Conditions: asm_file is non-null (writer_init has been called)
** Post-Conditions: Bootstrap code has been written to asm_file,
**     current_vm_file_name has been set to "Sys"
*******************************************************************************/
void write_bootstrap() {
    assert_nonnull(asm_file, "Error: Cannot write bootstrap to uninitialized "
            "assembly file\n");
    fprintf(asm_file, 
            "// bootstrap code\n"
            "@256\n"
            "D=A\n"
            "@SP\n"
            "M=D\n"
            "\n"
    );
    set_current_vm_file_name("Sys");
    write_asm_instructions("call Sys.init 0");
}

/*******************************************************************************
** Function: set_current_vm_file_name
** Description: Sets current_vm_file_name to provided name
** Parameters:
**     - vm_file_name: name of vm file currently being compiled
** Pre-Conditions: vm_file_name is non-null
** Post-Conditions: N/A
*******************************************************************************/
void set_current_vm_file_name(const char *vm_file_name) {
    strcpy(current_vm_file_name, vm_file_name);
}

/*******************************************************************************
** Function: write_asm_instructions
** Description: Writes the compiled assembly code for a given line of vm code
** Parameters:
**     - vm_line: line of vm code to compile
** Pre-Conditions: vm_line is non-null
** Post-Conditions: Compiled assembly instructions have been writen to asm_file
*******************************************************************************/
void write_asm_instructions(const char *vm_line) {
    assert_nonnull(vm_line, "Error: Cannot compile NULL vm line\n");
    write_vm_comment(vm_line);
    char operator[MAX_OPERATOR_LEN];
    cmd_args args;
    sscanf(vm_line, "%s %s %u", operator, args.operand, &(args.value));
    const vm_command *cmd = get_vm_command(operator);
    cmd->write_function(&args);
    fprintf(asm_file, "\n");
}

/*******************************************************************************
** Function: write_vm_comment
** Description: Writes a comment containing the contents of vm_line to asm_file
** Parameters:
**     - vm_line: Line of vm code to write a comment for
** Pre-Conditions: vm_line is non-null
** Post-Conditions: Comment containing vm_line has been written to asm_file
*******************************************************************************/
static void write_vm_comment(const char *vm_line) {
    fprintf(asm_file, "// %s\n", vm_line);
}

/*******************************************************************************
** Function: get_vm_command
** Description: Returns vm_command matching the given operator
** Parameters:
**     - operator: name of the vm command to match
** Pre-Conditions: N/A
** Post-Conditions: Return value is a valid vm_command
*******************************************************************************/
static vm_command *get_vm_command(const char *operator) {
    for (int cmd_index = 0; cmd_index < NUM_VM_OPERATORS; cmd_index++) {
        if (strcmp(operator, cmd_list[cmd_index].name) == EXIT_SUCCESS) {
            return cmd_list + cmd_index;
        }
    }
    fprintf(stderr, "Error: invalid operator \"%s\"\n", operator);
    exit(EXIT_FAILURE);
}

/*******************************************************************************
** Function: write_push
** Description: Writes compiled asm code for vm push operator
** Parameters:
**     - args->operand: Memory segment to push from
**     - args->value: Address offset from beginning of segment
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: Push asm instructions have been writen
*******************************************************************************/
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
                current_vm_file_name, args->value
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

/*******************************************************************************
** Function: write_push_segment
** Description: Writes push asm instructions for LCL, ARG, THIS, and THAT
**     segnments
** Parameters:
**     - segment: Memory segment to push from
**     - value: Adddress offset from begining of segment
** Pre-Conditions: segment is non-null
** Post-Conditions: Push asm instructions have been writen
*******************************************************************************/
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

/*******************************************************************************
** Function: write_pop
** Description: Writes compiled asm code for vm pop operator
** Parameters:
**     - args->operand: Memory segment to pop to
**     - args->value: Address offset from beginning of segment
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: Pop asm instructions have been writen
*******************************************************************************/
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
                current_vm_file_name, args->value
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

/*******************************************************************************
** Function: write_pop_segment
** Description: Writes pop asm instructions for LCL, ARG, THIS, and THAT
**     segnments
** Parameters:
**     - segment: Memory segment to pop to
**     - value: Adddress offset from begining of segment
** Pre-Conditions: segment is non-null
** Post-Conditions: Pop asm instructions have been writen
*******************************************************************************/
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

/*******************************************************************************
** Function: write_add
** Description: Writes compiled asm code for vm add operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Add asm instructions have been writen
*******************************************************************************/
static void write_add(__attribute__((unused)) const cmd_args *args) {
    write_binary_operation('+');
}

/*******************************************************************************
** Function: write_sub
** Description: Writes compiled asm code for vm sub operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Sub asm instructions have been writen
*******************************************************************************/
static void write_sub(__attribute__((unused)) const cmd_args *args) {
    write_binary_operation('-');
}

/*******************************************************************************
** Function: write_neg
** Description: Writes compiled asm code for vm neg operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Neg asm instructions have been writen
*******************************************************************************/
static void write_neg(__attribute__((unused)) const cmd_args *args) {
    fprintf(asm_file,
            "@SP\n"
            "A=M-1\n"
            "M=-M\n"
    );
}

/*******************************************************************************
** Function: write_eq
** Description: Writes compiled asm code for vm eq operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Eq asm instructions have been writen
*******************************************************************************/
static void write_eq(__attribute__((unused)) const cmd_args *args) {
    static int eq_count = 0;
    write_comparison("JEQ", eq_count);
    eq_count++;
}

/*******************************************************************************
** Function: write_lt
** Description: Writes compiled asm code for vm lt operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Lt asm instructions have been writen
*******************************************************************************/
static void write_lt(__attribute__((unused)) const cmd_args *args) {
    static int lt_count = 0;
    write_comparison("JLT", lt_count);
    lt_count++;
}

/*******************************************************************************
** Function: write_gt
** Description: Writes compiled asm code for vm gt operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Gt asm instructions have been writen
*******************************************************************************/
static void write_gt(__attribute__((unused)) const cmd_args *args) {
    static int gt_count = 0;
    write_comparison("JGT", gt_count);
    gt_count++;
}

/*******************************************************************************
** Function: write_and
** Description: Writes compiled asm code for vm and operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: And asm instructions have been writen
*******************************************************************************/
static void write_and(__attribute__((unused)) const cmd_args *args) {
    write_binary_operation('&');
}

/*******************************************************************************
** Function: write_or
** Description: Writes compiled asm code for vm or operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Or asm instructions have been writen
*******************************************************************************/
static void write_or(__attribute__((unused)) const cmd_args *args) {
    write_binary_operation('|');
}

/*******************************************************************************
** Function: write_not
** Description: Writes compiled asm code for vm not operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Not asm instructions have been writen
*******************************************************************************/
static void write_not(__attribute__((unused)) const cmd_args *args) {
    fprintf(asm_file,
            "@SP\n"
            "A=M-1\n"
            "M=!M\n"
    );
}

/*******************************************************************************
** Function: write_label
** Description: Writes compiled asm code for vm label operator
** Parameters:
**     - args->operand: Label to write
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: Label asm instruction has been writen
*******************************************************************************/
static void write_label(const cmd_args *args) {
    fprintf(asm_file, "(%s)\n", args->operand);
}

/*******************************************************************************
** Function: write_goto
** Description: Writes compiled asm code for vm goto operator
** Parameters:
**     - args->operand: Label to jump to
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: Goto asm instructions have been writen
*******************************************************************************/
static void write_goto(const cmd_args *args) {
    fprintf(asm_file,
            "@%s\n"
            "0;JMP\n",
            args->operand
    );
}

/*******************************************************************************
** Function: write_if_goto
** Description: Writes compiled asm code for vm if-goto operator
** Parameters:
**     - args->operand: Label to jump to if top of stack is non-zero
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: If-goto asm instructions have been writen
*******************************************************************************/
static void write_if_goto(const cmd_args *args) {
    fprintf(asm_file,
            POP_D
            "@%s\n"
            "D;JNE\n",
            args->operand
    );
}

/*******************************************************************************
** Function: write_call
** Description: Writes compiled asm code for vm call operator
** Parameters:
**     - args->operand: Name of function to call
**     - args->value: Number of arguments in called function
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: Call asm instruction has been writen
*******************************************************************************/
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

/*******************************************************************************
** Function: write_function
** Description: Writes compiled asm code for vm function operator
** Parameters:
**     - args->operand: Name of function to call
**     - args->value: Number of local variables in function
** Pre-Conditions: args and args->operand are non-null
** Post-Conditions: Function asm instruction has been writen
*******************************************************************************/
static void write_function(const cmd_args *args) {
    static const cmd_args constant_zero = {"constant", 0};
    const char *function_name = args->operand;
    int num_vars = args->value;
    fprintf(asm_file, "(%s)\n", function_name);
    for (int i = 0; i < num_vars; i++) {
        write_push(&constant_zero);
    }
}

/*******************************************************************************
** Function: write_return
** Description: Writes compiled asm code for vm return operator
** Parameters: unused
** Pre-Conditions: N/A
** Post-Conditions: Return asm instruction has been writen
*******************************************************************************/
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

/*******************************************************************************
** Function: write_binary_operation
** Description: Writes compiled assembly code to perform a binary operation,
**     one of add (+), sub (-), and (&), or or (|)
** Parameters:
**     - binary_operator: Symbol corresponding to relevant binary operation
** Pre-Conditions: N/A
** Post-Conditions: Asm instructions for relavant binary operation have been 
**     written
*******************************************************************************/
static void write_binary_operation(const char binary_operator) {
    fprintf(asm_file,
            POP_D
            "A=A-1\n"
            "M=M%cD\n",
            binary_operator
    );
}

/*******************************************************************************
** Function: write_comparison
** Description: Writes compiled assembly code to perform a binary comparison
**     one of eq (=), lt (<), or gt (>)
** Parameters:
**     - jump_code: Jump directive if comarison is a success (JEQ, JLT, or JGT)
** Pre-Conditions: jump_code is non-null
** Post-Conditions: Asm instructions for relavant comparison have been written
*******************************************************************************/
static void write_comparison(const char *jump_code, int count) {
    char *label = safe_malloc(MAX_OPERATOR_LEN * sizeof(char));
    sprintf(label, "%s%d", jump_code, count);
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
            label, jump_code, label
    );
    free(label);
}

/*******************************************************************************
** Function: get_return_label
** Description: Returns a string containing the name of the calling function
**     and the number of times a function has been called from within it
** Parameters:
**     - function_name: Name of calling function
** Pre-Conditions: function_name is non-null
** Post-Conditions: Return value is allocated memory and non-null
*******************************************************************************/
static char *get_return_label(const char *function_name) {
    unsigned call_count;
    if (hash_table_contains(call_counts, function_name)) {
        call_count = *((unsigned *) hash_table_get(call_counts, function_name));
        ++call_count;
        hash_table_set(call_counts, function_name, &call_count);
    } else {
        call_count = 0;
        hash_table_add(call_counts, function_name, &call_count);
    }
    size_t return_label_len = (strlen(function_name) + 5 /* strlen("$ret.") */
            + CALL_COUNT_LEN) * sizeof(char);
    char *return_label = safe_malloc(return_label_len);
    sprintf(return_label, "%s$ret.%0*d", function_name, CALL_COUNT_LEN,
            call_count);
    return return_label;
}

