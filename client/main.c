#include <signal.h>
#include <stdio.h>

#include "common.h"
#include "device.h"

static device_t* device_ptr;

void
interrupt()
{
  dvap_stop(device_ptr);
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

  if (!set_runstate(&ctx, DVAP_RUN_STATE_RUN)) {
    fprintf(stderr, "Error setting DVAP run state to RUN\n");
  }

  // Block until dvap_stop() is called
  dvap_wait(&ctx);
  return 0;
}
