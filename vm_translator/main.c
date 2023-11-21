#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "error_check.h"
#include "linked_list.h"
#include "asm_writer.h"
#include "parser.h"

#define VM_EXTENSION_LEN  3
#define ASM_EXTENSION_LEN 4
#define VM_EXTENSION  ".vm"
#define ASM_EXTENSION ".asm"
#define NULL_TERMINAOTR_LEN 1
#define MAX_VM_LINE   80
#define MAX_ASM_LINE 160

typedef enum { VM_FILE, DIRECTORY } file_type;
typedef struct {
  char *dir_absolute_path;
  char *file_absolute_path;
  file_type type;
} input_info;

static input_info parse_input_info(int argc, char **argv);
static node *get_vm_file_names(const input_info input);
static char *get_root_file_name(const input_info input);
static char *get_parent_dir_path(const char *path);
static char *get_asm_file_path(const input_info input);
static void generate_asm(FILE *vm_file);
static bool is_dir(const char *path);
static bool is_vm_file(const char *file_path);

int main(int argc, char *argv[]) {
  const input_info input = parse_input_info(argc, argv);
  node *vm_file_names_start = get_vm_file_names(input);
  node *vm_file_names = vm_file_names_start;
  char *asm_file_name = get_asm_file_path(input);
  FILE *asm_file = fopen(asm_file_name, "w");
  assert_file_opened(asm_file, asm_file_name);
  char *root_file_name = get_root_file_name(input);
  writer_init(asm_file, root_file_name);
  while (vm_file_names) {
    const char *vm_file_name = (char *) vm_file_names->data;
    FILE *vm_file = fopen(vm_file_name, "r");
    assert_file_opened(vm_file, vm_file_name);
    generate_asm(vm_file);
    assert_condition(fclose(vm_file) == EXIT_SUCCESS, "Error closing file %s\n",
        vm_file_name);
    vm_file_names = vm_file_names->next;
  }
  list_dispose(vm_file_names_start);
  assert_condition(fclose(asm_file) == EXIT_SUCCESS, "Error closing file %s\n",
      asm_file_name);
  free(asm_file_name);
  free(input.file_absolute_path);
  free(input.dir_absolute_path);

  return EXIT_SUCCESS;
}

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
    fprintf(stderr, "Error: invalid directory or .vm file: %s\n", relative_path);
    exit(EXIT_FAILURE);
  }
  return input;
}

static node *get_vm_file_names(const input_info input) {
  node *list_head = NULL;
  node *list_tail = NULL;
  DIR *dir;
  switch (input.type) {
    case DIRECTORY:
      dir = opendir(input.dir_absolute_path);
      assert_dir_opened(dir, input.dir_absolute_path);
      struct dirent *dir_entry;
      while ((dir_entry = readdir(dir)) != NULL) {
        char *file_path = dir_entry->d_name;
        if (is_vm_file(file_path)) {
          printf("%s\n", file_path);
          char *file_path_copy = strdup(file_path);
          assert_malloc_success((void *) file_path_copy);
          list_tail = list_append(list_tail, (void *) file_path_copy);
          list_head = list_head ? list_head : list_tail;
        }
      }
      assert_condition(closedir(dir) == EXIT_SUCCESS, 
          "Error closing directory %s\n", input.dir_absolute_path);
      break;
    case VM_FILE:
      printf("%s\n", input.file_absolute_path);
      char *file_path_copy = strdup(input.file_absolute_path);
      assert_malloc_success((void *) file_path_copy);
      list_tail = list_append(list_tail, (void *) file_path_copy);
      list_head = list_tail;
      break;
  }
  return list_head;
}

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
  printf("Root file name: %s\n", root_file_name);
  return root_file_name;
}

static char *get_parent_dir_path(const char *path) {
  const char *parent_dir_end = strrchr(path, '/') - 1;
  size_t parent_dir_path_len = parent_dir_end - path + NULL_TERMINAOTR_LEN;
  char *parent_dir_path = (char *) malloc(parent_dir_path_len * sizeof(char));
  assert_malloc_success((void *) parent_dir_path);
  strncpy(parent_dir_path, path, parent_dir_path_len);
  return parent_dir_path;
}

static char *get_asm_file_path(const input_info input) {
  static char *asm_file_path = NULL;
  size_t asm_path_len;
  const char *root_file_name = get_root_file_name(input);
  size_t root_name_len = strlen(root_file_name);
  size_t input_absolute_path_len = strlen(input.dir_absolute_path);
  asm_path_len = input_absolute_path_len + 1 + root_name_len + 
      ASM_EXTENSION_LEN + NULL_TERMINAOTR_LEN;
  asm_file_path = (char *) malloc(asm_path_len * sizeof(char));
  assert_malloc_success((void *) asm_file_path);
  sprintf(asm_file_path, "%s/%s%s", input.dir_absolute_path,
      root_file_name, ASM_EXTENSION);
  printf("%s\n", asm_file_path);
  return asm_file_path;
}

static bool is_dir(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != EXIT_SUCCESS) {
    return false;
  }
  return S_ISDIR(path_stat.st_mode);
}

static bool is_vm_file(const char *file_path) {
  size_t len = strlen(file_path);
  const char *extension_start = file_path + len - VM_EXTENSION_LEN;
  return (strncmp(extension_start, VM_EXTENSION, VM_EXTENSION_LEN) == 0);
}

static void generate_asm(FILE *vm_file) {
  static char vm_line[MAX_VM_LINE];
  while ((get_line(vm_line, MAX_VM_LINE, vm_file)) != NULL) {
    write_asm_instructions(vm_line);
  }
}

