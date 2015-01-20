#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define debug_print(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__,         \
                          __LINE__, __func__, __VA_ARGS__); } while (0)
#define DEBUG 1

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

void hex_dump(char* prefix, unsigned char* buf, int buf_len);

#endif
