#include <stdio.h>
#include "device.h"

int 
main(int argc, char* argv[])
{
  char buf[20];
  device_t ctx;

  if (dvap_init(&ctx) != 0) {
    fprintf(stderr, "Error initializing DVAP device\n");
    return -1;
  }

  if (!get_name(&ctx, buf, 20)) {
    printf("Name: %s\n", buf);
  }
  return 0;
}
