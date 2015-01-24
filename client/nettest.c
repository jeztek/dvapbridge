#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "network.h"
#include "common.h"

#define PORT 8001

int main(int argc, char* argv[])
{
  network_t ctx;
  char buf[4];
  buf[0] = 0xde;
  buf[1] = 0xad;
  buf[2] = 0xbe;
  buf[3] = 0xef;

  if (argc < 2) {
    printf("Usage: %s <hostname>\n", argv[0]);
    return -1;
  }

  if (!net_connect(&ctx, argv[1], PORT)) {
    fprintf(stderr, "Error connecting to %s on port %d\n", argv[1], PORT);
    return -1;
  }
  printf("Connected to %s on port %d\n", argv[1], PORT);

  send(ctx.fd, buf, 4, 0);
  close(ctx.fd);
  return 0;
}
