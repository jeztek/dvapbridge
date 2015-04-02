#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "common.h"
#include "device_gmsk.h"
#include "network.h"

#define PORT 8001

static network_t* network_ptr;
FILE* fp;

void
interrupt()
{
  net_stop(network_ptr);
  if (fp) {
    fclose(fp);
  }
}

void net_rx_callback(unsigned char* buf, int buf_bytes)
{
  unsigned int header;
  int write_bytes;
  int n;
  //hex_dump("net rx", buf, buf_bytes);
  if (buf_bytes < 2) {
    fprintf(stderr, "uh oh...\n");
    return;
  }

  if (fp) {
    write_bytes = 0;
    while (write_bytes < buf_bytes) {
      n = fwrite(&buf[write_bytes], 1, buf_bytes-write_bytes, fp);
      if (n <= 0) {
        fprintf(stderr, "Error writing to file\n");
        return;
      }
      write_bytes += n;
    }
    return;
  }

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
    break;
  }
}

int main(int argc, char* argv[])
{
  network_t ctx;

  if (argc < 2) {
    printf("Usage: %s <hostname [<dump file>]>\n", argv[0]);
    return -1;
  }

  if (argc == 3) {
    fp = fopen(argv[2], "wb");
    if (!fp) {
      fprintf(stderr, "Error opening file %s for output\n", argv[2]);
      return -1;
    }
  }

  network_ptr = &ctx;
  signal(SIGINT, interrupt);

  if (!net_init(&ctx, argv[1], PORT, &net_rx_callback)) {
    fprintf(stderr, "Error connecting to %s on port %d\n", argv[1], PORT);
    return -1;
  }
  printf("Connected to %s on port %d\n", argv[1], PORT);
  
  net_wait(&ctx);
  return 0;
}
