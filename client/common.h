#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define debug_print(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__,         \
                          __LINE__, __func__, __VA_ARGS__); } while (0)
#define DEBUG 1

void hex_dump(char* prefix, char* buf, int buf_len);

#endif
