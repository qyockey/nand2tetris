#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

static void remove_comments(const char *vm_line);
static void remove_newlines(char *vm_line);

/*******************************************************************************
** Function: get_line
** Description: Gets the next line from the provided vm file, or returns null
**     once the end of the file is reached
** Parameters:
**     - vm_line: buffer to copy line contents into
**     - max_line: maximum vm line length allowed
**     - vm_file: file to read from
** Pre-Conditions:
**     - vm_file is non-null
**     - max_line > 0
** Post-Conditions: vm_line contains the read line data, no more than max_line
**     bytes in length
*******************************************************************************/
char *get_line(char *vm_line, int max_line, FILE *vm_file) {
    do {
        if (fgets(vm_line, max_line, vm_file) == NULL) {
            return NULL;
        } 
        remove_comments(vm_line);
        remove_newlines(vm_line);
    } while (strlen(vm_line) == 0);
    return vm_line;
}

/*******************************************************************************
** Function: remove_comments
** Description: Replaces the start of an inline comment with a null terminator
** Parameters:
**     - vm_line: string to remove comments from
** Pre-Conditions: vm_line is non-null
** Post-Conditions: N/A
*******************************************************************************/
static void remove_comments(const char *vm_line) {
    char *comment_start = strstr(vm_line, "//");
    if (comment_start) {
        *comment_start = '\0';
    }
}

/*******************************************************************************
** Function: remove_newlines
** Description: Replaces the first newline or carriage return charater in a
**     string with a null terminator
** Parameters:
**     - vm_line: string to remove newlines from
** Pre-Conditions: vm_line is non-null
** Post-Conditions: N/A
*******************************************************************************/
static void remove_newlines(char *vm_line) {
    while ((*vm_line) != '\0') {
        if (*vm_line == '\r' || *vm_line == '\n') {
            *vm_line = '\0';
        }
        vm_line++;
    }
}

