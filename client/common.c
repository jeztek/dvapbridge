#include <stdio.h>
#include "common.h"

void
hex_dump(char* prefix, unsigned char* buf, int buf_len)
{
  int i;
  printf("%s: ", prefix);
  for (i = 0; i < buf_len; i++) {
    printf("%02X ", buf[i]);
  }
  printf("(%d bytes)\n", buf_len);
}
