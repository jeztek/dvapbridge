#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

// Set to 0 to disable, 1 to enable
#define DEBUG 0

// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define debug_print(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__,         \
                          __LINE__, __func__, __VA_ARGS__); } while (0)

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

void sleep_ms(int milliseconds);
void hex_dump(char* prefix, unsigned char* buf, int buf_len);

// Read DVAP packet - shared by device and network code
int packet_read(int fd, char* msg_type, unsigned char* buf, int buf_bytes);

#endif
