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

  /*
    Dump from DVAPTool program
    [get name]        tx: 04 20 01 00      rx: ...
    [get serial]      tx: 04 20 02 00 rx: 00 02 00 41 50 30 37 32 39 38 38 00

    [get hw ver]      tx: 05 20 04 00 00   rx: 07 00 04 00 00 6d 00
    [get hw ver]      tx: 05 20 04 00 01   rx: 07 00 04 00 01 6b 00
    [get tx freq lim] tx: 04 20 30 02 rx: 0c 00 30 02 00 44 95 08 00 4d d2 08
    [get rx freq]     tx: 04 20 20 00      rx: 08 00 20 00 70 bf ae 08
    [get tx freq]     tx: 04 20 20 01      rx: 08 00 20 01 70 bf ae 08
    [set rx freq]     tx: 08 00 20 00 70 bf ae 08 rx: 08 00 20 00 70 bf ae 08
    [set tx freq]     tx: 08 00 20 01 70 bf ae 08 rx: 08 00 20 01 70 bf ae 08
    [set mod type]    tx: 05 00 28 00 01   rx: 05 00 28 00 01
    [set run state]   tx: 05 00 18 00 01   rx: 05 00 18 00 01
    ...
    [set run state]   tx: 05 00 18 00 00   rx: 05 00 18 00 00
   */

  //set_operation_mode(&ctx, DVAP_OPERATION_NORMAL);
  //set_squelch_threshold(&ctx, -80);
  //set_tx_power(&ctx, -12);
  set_rx_frequency(&ctx, 145670000);
  set_tx_frequency(&ctx, 145670000);
  set_modulation_type(&ctx, DVAP_MODULATION_GMSK);

  if (!dvap_start(&ctx)) {
    fprintf(stderr, "Error starting DVAP device\n");
    return -1;
  }

  // Block until dvap_stop() is called
  dvap_wait(&ctx);
  return 0;
}
