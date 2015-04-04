#include <stdio.h>

#include "common.h"

int rx(FILE* fp)
{
  unsigned char buf[8191];
  int expected_bytes = 2;
  int received_bytes = 0;
  int n;

  n = fread(buf, 1, 2, fp);
  if (n <= 0) {
    if (feof(fp)) {
      return 0;
    }
    fprintf(stderr, "Error reading header\n");
    return -1;
  }
  received_bytes += n;
  
  expected_bytes = buf[0] + ((buf[1] & 0x1F) << 8);
  if (expected_bytes >= 8191) {
    fprintf(stderr, "Expected %d bytes, buf only 8191 bytes available\n", expected_bytes);
    return -1;
  }
  
  while (received_bytes < expected_bytes) {
    n = fread(&buf[received_bytes], 1, expected_bytes-received_bytes, fp);
    if (n <= 0) {
      if (feof(fp)) {
        break;
      }
      fprintf(stderr, "read returned %d\n", n);
      break;
    }
    received_bytes += n;
  }
  
  hex_dump("dump", buf, received_bytes);
  return 0;
}

int main(int argc, char* argv[])
{
  FILE* fp;

  if (argc < 2) {
    printf("Usage: %s <data file>\n", argv[0]);
    return -1;
  }

  fp = fopen(argv[1], "rb");
  if (!fp) {
    fprintf(stderr, "Error opening file %s\n", argv[1]);
    return -1;
  }

  while(1) {
    rx(fp);
    if (feof(fp)) {
      break;
    }
  }

  return 0;
}
