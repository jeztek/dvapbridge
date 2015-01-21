#include <signal.h>
#include <stdio.h>

#include "common.h"
#include "device.h"

static device_t* device_ptr;

void
interrupt()
{
  if (!dvap_stop(device_ptr)) {
    fprintf(stderr, "Error stopping DVAP device\n");
  }
}

int 
main(int argc, char* argv[])
{
  char buf[20];
  device_t ctx;

  // Configure CTRL+C handler
  device_ptr = &ctx;
  signal(SIGINT, interrupt);

  if (!dvap_init(&ctx)) {
    fprintf(stderr, "Error initializing DVAP device\n");
    return -1;
  }

  if (get_name(&ctx, buf, 20)) {
    printf("Name: %s\n", buf);
  }

  if (!dvap_start(&ctx)) {
    fprintf(stderr, "Error starting DVAP device\n");
    return -1;
  }

  // Block until dvap_stop() is called
  dvap_wait(&ctx);
  return 0;
}
