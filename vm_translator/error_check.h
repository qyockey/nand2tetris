#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
# define assert_condition(condition, ...) \
  do { \
    assert(condition); \
  } while (0)
#else
# define assert_condition(condition, ...) \
  do { \
    if (!(condition)) { \
      fprintf(stderr, __VA_ARGS__); \
      exit(EXIT_FAILURE); \
    } \
  } while (0)
#endif

void assert_dir_opened(const DIR *dir, const char *dir_name);
void assert_dir_closed(const DIR *dir, const char *dir_name);
void assert_file_opened(const FILE *file, const char *file_name);
void assert_file_closed(const FILE *file, const char *file_name);
void assert_malloc_success(void *pointer);

