#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

#if DEBUG
#   define print_failure_info() \
        printf("Execution halted at %s:%d\n", __FILE__, __LINE__)
#else
#   define print_failure_info()
#endif

#define assert_condition(condition, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, __VA_ARGS__); \
            print_failure_info(); \
            exit(EXIT_FAILURE); \
        } \
    } while (false)

#define assert_nonnull(ptr, ...) \
    assert_condition((ptr) != NULL, __VA_ARGS__)

#define assert_success(return_val, ...) \
    assert_condition((return_val) == EXIT_SUCCESS, __VA_ARGS__)

void *safe_malloc(size_t size);
char *safe_strdup(const char *string);
char *safe_realpath(const char *path, char *resolved_path);

FILE *safe_fopen(const char *file_path, const char *mode);
void safe_fclose(FILE *file);
DIR *safe_opendir(const char *dir_path);
void safe_closedir(DIR *dir);


