/*******************************************************************************
** Program Filename: main.c
** Author: Quinn Yockey
** Date: November 2023
** Description: Compile given .vm file(s) containing virtual machine
**     code as specified in the Nand2Tetris course into assembly
**     code to run on the Hack computer
** Input: a filepath to either
**     - a file with a .vm extension
**     - a directory containing one or more .vm files
** Output: a .asm file containing the compiled assembly code to run
**     on the Hack computer
*******************************************************************************/

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "asm_writer.h"
#include "error_check.h"
#include "hash_table.h"
#include "linked_list.h"
#include "parser.h"

#define VM_EXTENSION_LEN  3
#define ASM_EXTENSION_LEN 4
#define VM_EXTENSION  ".vm"
#define ASM_EXTENSION ".asm"
#define NULL_TERMINAOTR_LEN 1
#define MAX_VM_LINE   80
#define MAX_ASM_LINE 160

// file type of input
typedef enum { VM_FILE, DIRECTORY } file_type;

// information about input file(s)
typedef struct {
    char *dir_absolute_path;
    char *file_absolute_path;
    file_type type;
} input_info;

static input_info parse_input_info(int argc, char **argv);
static linked_list *get_vm_file_names(const input_info input);
static char *get_root_file_name(const input_info input);
static char *get_parent_dir_path(const char *path);
static char *get_asm_file_path(const input_info input);
static void generate_asm(FILE *vm_file);
static bool is_dir(const char *path);
static bool is_vm_file(const char *file_path);

/*******************************************************************************
** Function: main
** Description: execute compilation on provided vm file(s)
** Parameters:
**     - argc: number of provided command-line arguments
**     - argv: list of provided comand-line arguments
** Pre-Conditions: N/A
** Post-Conditions: compiled assembly code is written to .asm file
*******************************************************************************/
int main(int argc, char *argv[]) {
    const input_info input = parse_input_info(argc, argv);
    linked_list *vm_file_names = get_vm_file_names(input);
    char *asm_file_path = get_asm_file_path(input);
    printf("Writing to output file \"%s\"\n", asm_file_path);
    FILE *asm_file = fopen(asm_file_path, "w");
    assert_file_opened(asm_file, asm_file_path);
    char *root_file_name = get_root_file_name(input);
    writer_init(asm_file, root_file_name);
    free(root_file_name);
    list_node *node = vm_file_names->head;
    while (node) {
        const char *vm_file_name = node->data;
        printf("Compiling vm file \"%s\"\n", vm_file_name);
        FILE *vm_file = fopen(vm_file_name, "r");
        assert_file_opened(vm_file, vm_file_name);
        generate_asm(vm_file);
        assert_condition(fclose(vm_file) == EXIT_SUCCESS,
                "Error closing file %s\n", vm_file_name);
        node = node->next;
    }
    list_dispose(vm_file_names);
    assert_condition(fclose(asm_file) == EXIT_SUCCESS,
            "Error closing file %s\n", asm_file_path);
    free(asm_file_path);
    free(input.file_absolute_path);
    free(input.dir_absolute_path);
    printf("Compilation finished successfully\n");

    return EXIT_SUCCESS;
}

/*******************************************************************************
** Function: parse_input_info
** Description: Parse the following data into an input_info structure
**     - type: if input path is a .vm file or a directory
**     - file_absolute_path: absolute path to provided input file or directory
**     - dir_asolute_path: absolute path to inputted directory, or directory 
**                 containing inputted file
**     If the given input is not a .vm file or directory path, exit with error
** Parameters:
**     - argc: Number of provided command-line arguments
**     - argv: List of provided comand-line arguments
** Pre-Conditions: N/A
** Post-Conditions: All necessary fields of input will be set according to 
**     input path. If inputted path is a directory, file_absolute_path will be
**     null, but all other fields will be non-null.
*******************************************************************************/
static input_info parse_input_info(int argc, char **argv) {
    input_info input;
    assert_condition(argc == 2, "Usage: $ vm_translator dir_or_vm_path\n");
    char *relative_path = argv[1];
    if (is_dir(relative_path)) {
        input.type = DIRECTORY;
        input.file_absolute_path = NULL;
        input.dir_absolute_path = realpath(relative_path, NULL);
    } else if (is_vm_file(relative_path)) {
        input.type = VM_FILE;
        input.file_absolute_path = realpath(relative_path, NULL);
        input.dir_absolute_path = get_parent_dir_path(input.file_absolute_path);
    } else {
        fprintf(stderr, "Error: invalid directory or .vm file: \"%s\"\n",
                relative_path);
        exit(EXIT_FAILURE);
    }
    return input;
}


/*******************************************************************************
** Function: get_vm_file_names
** Description: Return a singly-linked list containing absoute paths to all .vm
**     files to compile
** Parameters:
**     - input: Contains relevant fields of user input
** Pre-Conditions: Fields of input are properly set.
** Post-Conditions: Returned list is non-null and contains names of inputted .vm
**     file or names of all .vm files in inputted directory
*******************************************************************************/
static linked_list *get_vm_file_names(const input_info input) {
    linked_list *vm_file_names = new_linked_list();
    DIR *dir = NULL;
    char *file_path_copy = NULL;
    switch (input.type) {
        case DIRECTORY:
            dir = opendir(input.dir_absolute_path);
            assert_dir_opened(dir, input.dir_absolute_path);
            const struct dirent *dir_entry;
            char absolute_path[PATH_MAX];
            sprintf(absolute_path, "%s/", input.dir_absolute_path);
            char *file_name_start = absolute_path + strlen(absolute_path);
            while ((dir_entry = readdir(dir)) != NULL) {
                const char *file_name = dir_entry->d_name;
                sprintf(file_name_start, "%s", file_name);
                if (is_vm_file(absolute_path)) {
                    char *absolute_path_copy = strdup(absolute_path);
                    list_append(vm_file_names, absolute_path_copy);
                }
            }
            assert_condition(closedir(dir) == EXIT_SUCCESS, 
                    "Error closing directory \"%s\"\n", input.dir_absolute_path);
            break;
        case VM_FILE:
            file_path_copy = strdup(input.file_absolute_path);
            assert_malloc_success(file_path_copy);
            list_append(vm_file_names, file_path_copy);
            break;
    }
    return vm_file_names;
}

/*******************************************************************************
** Function: get_root_file_name
** Description: If input type is a .vm file, return the name of the file without
**     the extension. Otherwise if the input type is a directory, return the
**     name of the directory
** Parameters:
**     - input: Contains relevant fields of user input
** Pre-Conditions: Fields of input are properly set
** Post-Conditions: Return value is non-null
*******************************************************************************/
static char *get_root_file_name(const input_info input) {
    size_t root_len;
    static char *root_file_name = NULL;
    const char *dir_name;
    const char *file_name;
    switch (input.type) {
        case DIRECTORY:
            dir_name = strrchr(input.dir_absolute_path, '/') + 1;
            root_file_name = strdup(dir_name);
            assert_malloc_success((void *) root_file_name);
            break;
        case VM_FILE:
            file_name = strrchr(input.file_absolute_path, '/') + 1;
            root_len = strlen(file_name) - VM_EXTENSION_LEN;
            root_file_name = malloc(root_len * sizeof(char));
            assert_malloc_success((void *) root_file_name);
            strncpy(root_file_name, file_name, root_len);
            break;
    }
    return root_file_name;
}

/*******************************************************************************
** Function: get_parent_dir_path
** Description: Returns absolute path to the parent directory of the file or
**     directory specified in path
** Parameters:
**     - path: Absolute path to a file or directory
** Pre-Conditions: path is non-null and has at least one '/' character
** Post-Conditions: Return value is non-null
*******************************************************************************/
static char *get_parent_dir_path(const char *path) {
    char *parent_dir_path;
    // parent of root is root
    if (strcmp(path, "/") == EXIT_SUCCESS) {
        parent_dir_path = strdup("/");
        assert_malloc_success((void *) parent_dir_path);
        return parent_dir_path;
    }
    const char *parent_dir_end = strrchr(path, '/') - 1;
    assert_condition(parent_dir_end != NULL,
            "Error: invalid absolute path \"%s\"\n", path);
    size_t parent_dir_path_len = parent_dir_end - path + NULL_TERMINAOTR_LEN;
    parent_dir_path = (char *) malloc(parent_dir_path_len * sizeof(char));
    assert_malloc_success((void *) parent_dir_path);
    strncpy(parent_dir_path, path, parent_dir_path_len);
    return parent_dir_path;
}

/*******************************************************************************
** Function: get_asm_file_path
** Description: Returns the absolute path to the .asm output file
** Parameters:
**     - input: Contains relevant fields of user input
** Pre-Conditions: Fields of input are properly set
** Post-Conditions: Return value is non-null
*******************************************************************************/
static char *get_asm_file_path(const input_info input) {
    static char *asm_file_path = NULL;
    size_t asm_path_len;
    char *root_file_name = get_root_file_name(input);
    size_t root_name_len = strlen(root_file_name);
    size_t input_absolute_path_len = strlen(input.dir_absolute_path);
    asm_path_len = input_absolute_path_len + 1 + root_name_len + 
            ASM_EXTENSION_LEN + NULL_TERMINAOTR_LEN;
    asm_file_path = (char *) malloc(asm_path_len * sizeof(char));
    assert_malloc_success((void *) asm_file_path);
    sprintf(asm_file_path, "%s/%s%s", input.dir_absolute_path,
            root_file_name, ASM_EXTENSION);
    free(root_file_name);
    return asm_file_path;
}

/*******************************************************************************
** Function: is_dir
** Description: Returns true if the file located at path is a directory, or
**     flase if it is not
** Parameters:
**     - path: Relative or absolute path to file to test
** Pre-Conditions: path is non-null
** Post-Conditions: N/A
*******************************************************************************/
static bool is_dir(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != EXIT_SUCCESS) {
        // stat error
        perror(path);
        return false;
    }
    // is directory
    return S_ISDIR(path_stat.st_mode);
}

/*******************************************************************************
** Function: is_vm_file
** Description: Return true if file_path points to a valid .vm file
** Parameters: 
**     - file_path: Relative or absolute path to file to test
** Pre-Conditions: path is non-null
** Post-Conditions: N/A
*******************************************************************************/
static bool is_vm_file(const char *file_path) {
    size_t len = strlen(file_path);
    const char *extension_start = file_path + len - VM_EXTENSION_LEN;
    if (strncmp(extension_start, VM_EXTENSION, VM_EXTENSION_LEN) != 0) {
        // doesn't end in ".vm"
        return false;
    }
    struct stat path_stat;
    if (stat(file_path, &path_stat) != EXIT_SUCCESS) {
        // stat error
        perror(file_path);
        return false;
    }
    // is regular file
    return S_ISREG(path_stat.st_mode);
}

/*******************************************************************************
** Function: generate_asm
** Description: Writes the compiled assembly code for each line of the given
**     .vm file
** Parameters:
**     - vm_file: file to compile
** Pre-Conditions: vm_file is non-null
** Post-Conditions: N/A
*******************************************************************************/
static void generate_asm(FILE *vm_file) {
    static char vm_line[MAX_VM_LINE];
    while ((get_line(vm_line, MAX_VM_LINE, vm_file)) != NULL) {
        write_asm_instructions(vm_line);
    }
}

