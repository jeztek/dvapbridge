#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "device.h"
#include "device_gmsk.h"
#include "network.h"

#define PORT 8191
#define USE_DVAP 1

/* TODO:
 * Make dvap device single duplex via mutex lock?
 * What happens when audio arrives over network while transmitting?
 */

static network_t* network_ptr;
static device_t* device_ptr;

// Try connecting to server until user cancels
static int net_init_retry = TRUE;

// Handle CTRL+C
void
interrupt()
{
  net_init_retry = FALSE;
  net_stop(network_ptr, FALSE);
#if USE_DVAP
  if (!dvap_stop(device_ptr)) {
    fprintf(stderr, "Error stopping DVAP device\n");
  }
#endif
}

// Called when we receive data from network
void net_rx_callback(unsigned char* buf, int buf_bytes)
{
  unsigned int header;

  if (buf_bytes < 2) return;
  header = (buf[1] << 8) + buf[0];

  // Write packet to device then sleep the appropriate amount to
  // avoid overflowing DVAP's receive buffer
  dvap_pkt_write(device_ptr, buf, buf_bytes);
  sleep_ms(buf_bytes);
  return;

  switch(header) {
  // FM data
  case 0x8142:
    hex_dump("fm data", buf, 2);
    break;
  // GMSK header
  case 0xA02F:
    gmsk_parse_header(buf, buf_bytes);
    break;
  // GMSK data
  case 0xC012:
    if (buf_bytes < 4) return;
    //gmsk_parse_data(buf, buf_bytes);
    break;
  default:
    hex_dump("net rx unrecognized", buf, buf_bytes);
    break;
  }
}

// Called when we receive data from DVAP device
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
    gmsk_parse_header(buf, buf_len);
    net_write(network_ptr, buf, buf_len);
    break;
  // GMSK data
  case 0xC012:
    if (buf_len < 4) return;
    //gmsk_parse_data(buf, buf_len);
    net_write(network_ptr, buf, buf_len);
    break;
  default:
    hex_dump("dvap rx unrecognized", buf, buf_len);
    break;
  }
}

int
timeout_retry_wrapper(int argc, char* argv[])
{
  char buf[20];
  int net_init_success;

  // Attempt to initialize network, retry on failure until user cancels
  do {
    net_init_success = net_init(network_ptr, argv[1], PORT, &net_rx_callback);
    if (!net_init_success) {
      fprintf(stderr, "Error connecting to %s on port %d\n", argv[1], PORT);
      sleep(2);
    }
    if (!net_init_retry) {
      return -1;
    }
  }
  while(!net_init_success && net_init_retry);
  printf("Connected to %s on port %d\n", argv[1], PORT);

#if USE_DVAP
  // Initialize DVAP
  if (!dvap_init(device_ptr, argv[2], &dvap_rx_callback)) {
    fprintf(stderr, "No DVAP device found at %s\n", argv[2]);
    return -1;
  }

  if (get_name(device_ptr, buf, 20)) {
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

  //set_operation_mode(device_ptr, DVAP_OPERATION_NORMAL);
  //set_squelch_threshold(device_ptr, -80);
  //set_tx_power(device_ptr, -12);
  printf("cmd - set rx frequency\n");
  set_rx_frequency(device_ptr, 145670000);
  printf("cmd - set tx frequency\n");
  set_tx_frequency(device_ptr, 145670000);
  printf("cmd - set modulation type\n");
  set_modulation_type(device_ptr, DVAP_MODULATION_GMSK);

  printf("cmd - dvap start\n");
  if (!dvap_start(device_ptr)) {
    fprintf(stderr, "Error starting DVAP device\n");
    return -1;
  }
#endif

  // Block until net_read_loop finishes
  net_wait(network_ptr);

  // If net_read_loop finished due to a network timeout,
  // signal stop of dvap so we can restart
  if (network_ptr->try_restart) {
#if USE_DVAP
    dvap_stop(device_ptr);
#endif
    sleep(2);
  }

#if USE_DVAP
  // Block until dvap_read_loop finishes
  dvap_wait(device_ptr);
#endif

  return 0;
}

int
main(int argc, char* argv[])
{
  network_t n_ctx;
  device_t d_ctx;
  int ret;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <server> <device>\n", argv[0]);
    return -1;
  }

  // Configure CTRL+C handler
  network_ptr = &n_ctx;
  device_ptr = &d_ctx;
  signal(SIGINT, interrupt);

  do {
    ret = timeout_retry_wrapper(argc, argv);
  } while(n_ctx.try_restart);
  if (ret) return ret;

  return 0;
}
