#include <stdio.h>
#include <unistd.h>
#include "common.h"

void sleep_ms(int milliseconds)
{
  usleep(milliseconds * 1000);
}

void
hex_dump(char* prefix, unsigned char* buf, int buf_len)
{
  int i;
  printf("    %s: ", prefix);
  for (i = 0; i < buf_len; i++) {
    printf("%02X ", buf[i]);
  }
  printf("(%d bytes)\n", buf_len);
}

int
packet_read(int fd, char* msg_type, unsigned char* buf, int buf_bytes)
{
  int expected_bytes = 2;
  int received_bytes = 0;
  int n;

  if (buf_bytes < expected_bytes) {
    fprintf(stderr, "packet_read - buf_bytes too small\n");
    return -1;
  }

  // Receive header
  n = read(fd, &buf[0], expected_bytes);
  if (n <= 0) {
    debug_print("packet_read - returned %d\n", n);
    return n;
  }
  received_bytes += n;

  // Determine how much data to expect
  expected_bytes = buf[0] + ((buf[1] & 0x1F) << 8);
  if (expected_bytes >= buf_bytes) {
    fprintf(stderr, "packet_read - expected %d bytes but only %d bytes available\n", expected_bytes, buf_bytes);
    return -1;
  }

  // Receive rest of packet
  while (received_bytes < expected_bytes) {
    n = read(fd, &buf[received_bytes], expected_bytes-received_bytes);
    if (n <= 0) {
      debug_print("packet_read - returned %d\n", n);
      return n;
    }
    received_bytes += n;
  }

  if (msg_type) {
    *msg_type = (buf[1] & 0xE0) >> 5;
  }

  return received_bytes;
}
