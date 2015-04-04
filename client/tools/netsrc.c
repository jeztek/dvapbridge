// netsrc.c
// This utility send previously recorded data dumps back to the server

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "network.h"
#include "common.h"

#define PORT 8001

int read_packet(FILE* fp, unsigned char* buf, int buf_len)
{
  int expected_bytes = 2;
  int received_bytes = 0;
  int n;

  n = fread(buf, 1, 2, fp);
  if (n <= 0) {
    if (feof(fp)) {
      return n;
    }
    fprintf(stderr, "Error reading header\n");
    return n;
  }
  received_bytes += n;
  
  expected_bytes = buf[0] + ((buf[1] & 0x1F) << 8);
  if (expected_bytes >= buf_len) {
    fprintf(stderr, "Expected %d bytes, buf only %d bytes available\n", expected_bytes, buf_len);
    return -1;
  }
  
  while (received_bytes < expected_bytes) {
    n = fread(&buf[received_bytes], 1, expected_bytes-received_bytes, fp);
    if (n <= 0) {
      return n;
    }
    received_bytes += n;
  }
  return received_bytes;
}

int main(int argc, char* argv[])
{
  FILE* fp;
  unsigned char buf[8191];
  int send_bytes, sent_bytes;
  int n;
  network_t ctx;

  if (argc < 3) {
    printf("Usage: %s <hostname> <data file>\n", argv[0]);
    return -1;
  }
  
  fp = fopen(argv[2], "rb");
  if (!fp) {
    fprintf(stderr, "Error opening file %s\n", argv[2]);
    return -1;
  }

  if (!net_init(&ctx, argv[1], PORT, NULL)) {
    fprintf(stderr, "Error connecting to %s on port %d\n", argv[1], PORT);
    return -1;
  }
  printf("Connected to %s on port %d\n", argv[1], PORT);

  while(1) {
    send_bytes = read_packet(fp, buf, 8191);
    sent_bytes = 0;
    while (sent_bytes < send_bytes) {
      n = send(ctx.fd, &buf[sent_bytes], send_bytes-sent_bytes, 0);
      if (n <= 0) {
        fprintf(stderr, "Error sending data to server\n");
        return -1;
      }
      sent_bytes += n;
    }
    sleep_ms(send_bytes);
    if (feof(fp)) {
      break;
    }
  }

  close(ctx.fd);
  fclose(fp);
  return 0;
}
