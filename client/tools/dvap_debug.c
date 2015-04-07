// dvap_debug.c
// This utility is used to dump data received from a DVAP device and
// reply dumps back to the DVAP for transmission

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "device.h"
#include "device_gmsk.h"

static device_t* device_ptr;
FILE* fp;
int write_mode = FALSE;
int fread_stop = FALSE;

void
interrupt()
{
  if (!dvap_stop(device_ptr)) {
    fprintf(stderr, "Error stopping DVAP device\n");
  }
  fread_stop = TRUE;
}

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

void dvap_rx_callback(unsigned char* buf, int buf_len)
{
  int write_bytes = 0;
  int n;

  if (fp) {
    while (write_bytes < buf_len) {
      n = fwrite(&buf[write_bytes], 1, buf_len-write_bytes, fp);
      if (n <= 0) {
        fprintf(stderr, "Error writing to file\n");
        return;
      }
      write_bytes += n;
    }
    printf("Received %d bytes from DVAP\n", buf_len);
  }
}

void
print_usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <device> <option> <data file>\n", cmd);
  fprintf(stderr, "  -r receive from dvap and write to file\n");
  fprintf(stderr, "  -w read from file and write to dvap\n");
}

int
main(int argc, char* argv[])
{
  char buf[20];
  unsigned char dbuf[8191];
  int n;
  device_t device_ctx;
 
  if (argc < 4) {
    print_usage(argv[0]);
    return -1;
  }

  if (!strncmp("-r", argv[2], 2)) {
    fp = fopen(argv[3], "wb");
    if (!fp) {
      fprintf(stderr, "Error opening file %s\n", argv[3]);
      return -1;
    }
    write_mode = FALSE;
  }
  else if (!strncmp("-w", argv[2], 2)) {
    fp = fopen(argv[3], "rb");
    if (!fp) {
      fprintf(stderr, "Error opening file %s\n", argv[3]);
      return -1;
    }
    write_mode = TRUE;
  }
  else {
    print_usage(argv[0]);
    return -1;
  }

  device_ptr = &device_ctx;
  signal(SIGINT, interrupt);

  if (!dvap_init(&device_ctx, argv[1], &dvap_rx_callback)) {
    fprintf(stderr, "Error initializing DVAP device\n");
    return -1;
  }

  if (get_name(&device_ctx, buf, 20)) {
    printf("Device name: %s\n", buf);
  }

  printf("cmd - set rx frequency\n");
  set_rx_frequency(&device_ctx, 145670000);
  printf("cmd - set tx frequency\n");
  set_tx_frequency(&device_ctx, 145670000);
  printf("cmd - set modulation type\n");
  set_modulation_type(&device_ctx, DVAP_MODULATION_GMSK);

  printf("cmd - dvap start\n");
  if (!dvap_start(&device_ctx)) {
    fprintf(stderr, "Error starting DVAP device\n");
    return -1;
  }

  if (write_mode) {
    printf("Sending file data to DVAP\n");

    while(!fread_stop) {
      n = read_packet(fp, dbuf, 8191);
      printf("%d bytes\n", n);
      dvap_pkt_write(&device_ctx, dbuf, n);
      sleep_ms(n);
      if (feof(fp)) {
        break;
      }
    }
    printf("Done!\n");
    sleep(2);
    dvap_stop(&device_ctx);
  }

  // Block until dvap_stop() is called
  dvap_wait(&device_ctx);

  if (fp) {
    fclose(fp);
  }

  return 0;
}
