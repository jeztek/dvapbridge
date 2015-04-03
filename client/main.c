#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "device.h"
#include "device_gmsk.h"
#include "network.h"

#define PORT 8001

/* TODO:
 * Make dvap device single duplex via mutex lock?
 *   What happens when audio arrives over network while transmitting?
 * Server should properly route streams destined for a specific user
 */

static network_t* network_ptr;
static device_t* device_ptr;

void
interrupt()
{
  net_stop(network_ptr);
  if (!dvap_stop(device_ptr)) {
    fprintf(stderr, "Error stopping DVAP device\n");
  }
}

void net_rx_callback(unsigned char* buf, int buf_bytes)
{
  unsigned int header;
  dvap_pkt_write(device_ptr, buf, buf_bytes);
  return;

  if (buf_bytes < 2) return;
  header = (buf[1] << 8) + buf[0];

  switch(header) {
  // FM data
  case 0x8142:
    hex_dump("fm data", buf, 2);
    break;
  // GMSK header
  case 0xA02F:
    //hex_dump("gmsk header", buf, buf_bytes);
    gmsk_parse_header(buf, buf_bytes);
    break;
  // GMSK data
  case 0xC012:
    if (buf_bytes < 4) return;
    //hex_dump("gmsk data", buf, 4);
    gmsk_parse_data(buf, buf_bytes);
    break;
  default:
    fprintf(stderr, "dvap_rx_callback: unrecognized data\n");
    hex_dump("net rx", buf, buf_bytes);
    break;
  }
}

void dvap_rx_callback(unsigned char* buf, int buf_len)
{
  unsigned int header;
  if (buf_len < 2) return;
  header = (buf[1] << 8) + buf[0];

  switch(header) {
  // FM data
  case 0x8142:
    hex_dump("fm data", buf, 2);
    break;
  // GMSK header
  case 0xA02F:
    //hex_dump("gmsk header", buf, buf_len);
    gmsk_parse_header(buf, buf_len);
    net_write(network_ptr, buf, buf_len);
    break;
  // GMSK data
  case 0xC012:
    if (buf_len < 4) return;
    //hex_dump("gmsk data", buf, 4);
    gmsk_parse_data(buf, buf_len);
    net_write(network_ptr, buf, buf_len);
    break;
  default:
    fprintf(stderr, "dvap_rx_callback: unrecognized data\n");
    break;
  }
}

int 
main(int argc, char* argv[])
{
  char buf[20];
  network_t n_ctx;
  device_t d_ctx;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <server>\n", argv[0]);
    return -1;
  }

  // Configure CTRL+C handler
  network_ptr = &n_ctx;
  device_ptr = &d_ctx;
  signal(SIGINT, interrupt);

  if (!net_init(&n_ctx, argv[1], PORT, &net_rx_callback)) {
    fprintf(stderr, "Error connecting to %s on port %d\n", argv[1], PORT);
    return -1;
  }
  printf("Connected to %s on port %d\n", argv[1], PORT);

  if (!dvap_init(&d_ctx, &dvap_rx_callback)) {
    fprintf(stderr, "Error initializing DVAP device\n");
    return -1;
  }

  if (get_name(&d_ctx, buf, 20)) {
    printf("Device name: %s\n", buf);
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

  //set_operation_mode(&d_ctx, DVAP_OPERATION_NORMAL);
  //set_squelch_threshold(&d_ctx, -80);
  //set_tx_power(&d_ctx, -12);
  printf("cmd - set rx frequency\n");
  set_rx_frequency(&d_ctx, 145670000);
  printf("cmd - set tx frequency\n");
  set_tx_frequency(&d_ctx, 145670000);
  printf("cmd - set modulation type\n");
  set_modulation_type(&d_ctx, DVAP_MODULATION_GMSK);

  printf("cmd - dvap start\n");
  if (!dvap_start(&d_ctx)) {
    fprintf(stderr, "Error starting DVAP device\n");
    return -1;
  }

  // Block until net_stop() and dvap_stop() are called
  net_wait(&n_ctx);
  dvap_wait(&d_ctx);
  return 0;
}
