#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "error_check.h"

void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    assert_nonnull(ptr, "Error allocating memory");
    return ptr;
}

char *safe_strdup(const char *string) {
    char *duplicate = strdup(string);
    assert_nonnull(duplicate, "Error allocating memory");
    return duplicate;
}

char *safe_realpath(const char *path, char *resolved_path) {
    char *absolute_path = realpath(path, resolved_path);
    assert_nonnull(absolute_path, "Error allocating memory");
    return absolute_path;
}

FILE *safe_fopen(const char *file_path, const char *mode) {
    FILE *file = fopen(file_path, mode);
    assert_nonnull(file, "Error allocating memory");
    return file;
}

void safe_fclose(FILE *file) {
    assert_success(fclose(file), "Error closing file");
}

DIR *safe_opendir(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    assert_nonnull(dir, "Error allocating memory");
    return dir;
}

void safe_closedir(DIR *dir) {
    assert_success(closedir(dir), "Error closing directory");
}

