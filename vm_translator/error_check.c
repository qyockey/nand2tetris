#include "error_check.h"

void assert_dir_opened(const DIR *dir, const char *dir_name) {
    assert_condition(dir != NULL, "Error opening directory \"%s\"\n", dir_name);
}

void assert_dir_closed(const DIR *dir, const char *dir_name) {
    assert_condition(dir != NULL, "Error closing directory \"%s\"\n", dir_name);
}

void assert_file_opened(const FILE *file, const char *file_name) {
    assert_condition(file != NULL, "Error opening file \"%s\"\n", file_name);
}

void assert_file_closed(const FILE *file, const char *file_name) {
    assert_condition(file != NULL, "Error closing file \"%s\"\n", file_name);
}

void assert_malloc_success(const void *pointer) {
    assert_condition(pointer != NULL,
            "Error allocating memory with malloc, memory leak likely\n");
}

