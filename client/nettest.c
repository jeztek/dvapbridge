#include <stdio.h>
#include <unistd.h>
#include "network.h"
#include "common.h"

#define PORT 80

int main(int argc, char* argv[])
{
  network_t ctx;

  if (argc < 2) {
    printf("Usage: %s <hostname>\n", argv[0]);
    return -1;
  }

  if (!net_connect(&ctx, argv[1], PORT)) {
    fprintf(stderr, "Error connecting to %s on port %d\n", argv[1], PORT);
    return -1;
  }
  printf("Connected to %s on port %d\n", argv[1], PORT);

  close(ctx.fd);
  return 0;
}
