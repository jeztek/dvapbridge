#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "common.h"
#include "device.h"
#include "serial.h"

int
get_name(device_t* ctx, char* name, int name_len)
{
  int sent, ret;
  char msg_type;
  char buf[DVAP_MSG_MAX_BYTES];

  sent = dvap_write(ctx->fd, DVAP_MSG_HOST_REQ_CTRL_ITEM, 
                    DVAP_CTRL_TARGET_NAME, NULL, 0);
  if (sent <= 0) {
    debug_print("%s\n", "get_name: error on dvap_write");
    return -1;
  }
  ret = dvap_read(ctx->fd, &msg_type, buf, DVAP_MSG_MAX_BYTES);
  if (ret <= 0) {
    debug_print("%s\n", "get_name: error on dvap_read");
    return -1;
  }
  ret -= 4;
  ret = (ret <= name_len) ? ret : name_len;
  strncpy(name, &buf[4], ret);
  printf("msg_type: %d\n", msg_type);
  return 0;
}

int
dvap_init(device_t* ctx)
{
  int fd;
  if (!ctx) return -1;

  fd = serial_open("/dev/tty.usbserial-A602S3VR", B230400);
  if (fd < 0) {
    return -1;
  }
  ctx->fd = fd;

  return 0;
}

int
dvap_write(int fd, char msg_type, int command, char* payload, 
             int payload_bytes)
{
  int n;
  int sent_bytes = 0;
  char buf[4];
  int pktlen;
  
  if (payload_bytes > DVAP_MSG_MAX_BYTES-4) {
    fprintf(stderr, "Error in send_command, payload too long\n");
    return -1;
  }

  pktlen = 4 + payload_bytes;
  buf[0] = pktlen & 0xFF;
  buf[1] = (msg_type << 5) | ((pktlen & 0x1F00) >> 8);

  buf[2] = command & 0xFF;
  buf[3] = (command << 8) & 0xFF;

  if (DEBUG) {
    hex_dump("tx", buf, 4);
  }

  n = write(fd, &buf[0], 4);
  if (n <= 0) {
    debug_print("dvap_write - returned %d\n", n);
    return n;
  }
  sent_bytes += n;

  if (payload) {
    if (DEBUG) {
      hex_dump("tx payload", payload, payload_bytes);
    }
    n = write(fd, payload, payload_bytes);
    if (n <= 0) {
      debug_print("dvap_write - returned %d\n", n);
      return n;
    }
    sent_bytes += n;
  }

  return sent_bytes;
}

int
dvap_read(int fd, char* msg_type, char* buf, int buf_bytes)
{
  int expected_bytes = 2;
  int received_bytes = 0;
  int n;

  if (buf_bytes < expected_bytes) {
    fprintf(stderr, "dvap_read - buf_bytes too small\n");
    return -1;
  }

  // Receive header
  do {
    n = read(fd, &buf[received_bytes], buf_bytes-received_bytes);
    if (n <= 0) {
      debug_print("dvap_read - returned %d\n", n);
      return n;
    }
    received_bytes += n;
    if (DEBUG) {
      hex_dump("rx", buf, n);
    }
  }
  while (received_bytes < expected_bytes);

  // Determine how much data to expect
  expected_bytes = buf[0] + ((buf[1] & 0x1F) << 8);
  if (expected_bytes >= buf_bytes) {
    fprintf(stderr, "dvap_read - expected %d bytes but only %d bytes available\n", expected_bytes, buf_bytes);
    return -1;
  }

  // Receive rest of packet
  while (received_bytes < expected_bytes) {
    n = read(fd, &buf[received_bytes], buf_bytes-received_bytes);
    if (n <= 0) {
      debug_print("dvap_read - returned %d\n", n);
      return n;
    }
    received_bytes += n;
    if (DEBUG) {
      hex_dump("rx", &buf[received_bytes], n);
    }
  }

  if (msg_type) {
    *msg_type = (buf[1] & 0xE0) >> 5;
  }

  return received_bytes;
}
