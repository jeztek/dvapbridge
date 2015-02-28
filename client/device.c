#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"
#include "device.h"
#include "serial.h"

int
get_name(device_t* ctx, char* name, int name_len)
{
  int sent, ret;
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  sent = dvap_write(ctx, DVAP_MSG_HOST_REQ_CTRL_ITEM,
                    DVAP_CTRL_TARGET_NAME, NULL, 0);
  if (sent <= 0) {
    debug_print("%s\n", "get_name: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);
  ret -= 2;
  ret = (ret <= name_len) ? ret : name_len;
  strncpy(name, (const char *)&buf[2], ret);

  return TRUE;
}

int
set_run_state(device_t* ctx, char state)
{
  int sent, ret;
  unsigned char payload[1];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  payload[0] = state;
  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_RUN_STATE,
                    payload, 1);
  if (sent <= 0) {
    debug_print("%s\n", "set_run_state: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
set_modulation_type(device_t* ctx, char modulation)
{
  int sent, ret;
  unsigned char payload[1];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  payload[0] = modulation;
  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_MODULATION_TYPE,
                    payload, 1);
  if (sent <= 0) {
    debug_print("%s\n", "set_modulation_type: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
set_operation_mode(device_t* ctx, char mode)
{
  int sent, ret;
  unsigned char payload[1];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  payload[0] = mode;
  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_OPERATION_MODE,
                    payload, 1);
  if (sent <= 0) {
    debug_print("%s\n", "set_operation_mode: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
set_squelch_threshold(device_t* ctx, int dbm)
{
  int sent, ret;
  int squelch = dbm;
  unsigned char payload[1];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  squelch = (dbm < DVAP_SQUELCH_MIN) ? DVAP_SQUELCH_MIN : squelch;
  squelch = (dbm < DVAP_SQUELCH_MAX) ? DVAP_SQUELCH_MAX : squelch;

  payload[0] = squelch & 0xFF;
  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_SQUELCH_THRESH,
                    payload, 1);
  if (sent <= 0) {
    debug_print("%s\n", "set_squelch_threshold: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
set_rx_frequency(device_t* ctx, unsigned int hz)
{
  int sent, ret;
  unsigned char payload[4];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  payload[0] = hz & 0xFF;
  payload[1] = (hz >> 8) & 0xFF;
  payload[2] = (hz >> 16) & 0xFF;
  payload[3] = (hz >> 24) & 0xFF;

  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_RX_FREQ,
                    payload, 4);
  if (sent <= 0) {
    debug_print("%s\n", "set_rx_frequency: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
set_tx_frequency(device_t* ctx, unsigned int hz)
{
  int sent, ret;
  unsigned char payload[4];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  payload[0] = hz & 0xFF;
  payload[1] = (hz >> 8) & 0xFF;
  payload[2] = (hz >> 16) & 0xFF;
  payload[3] = (hz >> 24) & 0xFF;

  printf("Setting tx frequency %02X\n", DVAP_CTRL_TX_FREQ);
  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_TX_FREQ,
                    payload, 4);
  if (sent <= 0) {
    debug_print("%s\n", "set_tx_frequency: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
set_rxtx_frequency(device_t* ctx, unsigned int hz)
{
  int sent, ret;
  unsigned char payload[4];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  payload[0] = hz & 0xFF;
  payload[1] = (hz >> 8) & 0xFF;
  payload[2] = (hz >> 16) & 0xFF;
  payload[3] = (hz >> 24) & 0xFF;

  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_TX_RX_FREQ,
                    payload, 4);
  if (sent <= 0) {
    debug_print("%s\n", "set_rxtx_frequency: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int set_tx_power(device_t* ctx, int dbm)
{
  int sent, ret;
  int power;
  unsigned char payload[2];
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  if (!ctx) return FALSE;

  power = (dbm < DVAP_TX_POWER_MIN) ? DVAP_TX_POWER_MIN : dbm;
  power = (dbm > DVAP_TX_POWER_MAX) ? DVAP_TX_POWER_MAX : dbm;

  payload[0] = power & 0xFF;
  payload[1] = (power >> 8) & 0xFF;

  sent = dvap_write(ctx, DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_TX_RX_FREQ,
                    payload, 2);
  if (sent <= 0) {
    debug_print("%s\n", "set_tx_power: error on dvap_write");
    return FALSE;
  }

  queue_remove(&(ctx->rxq), buf, &ret);

  return TRUE;
}

int
dvap_init(device_t* ctx)
{
  int fd;
  if (!ctx) return FALSE;

  // serial port
  fd = serial_open("/dev/tty.usbserial-A602S3VR", B230400);
  if (fd < 0) {
    return FALSE;
  }
  ctx->fd = fd;

  // shutdown variables
  ctx->shutdown = FALSE;
  pthread_mutex_init(&(ctx->shutdown_mutex), NULL);

  pthread_mutex_init(&(ctx->tx_mutex), NULL);

  // receive queue
  queue_init(&(ctx->rxq));
  pthread_create(&(ctx->rx_thread), NULL, read_loop, ctx);

  return TRUE;
}

// Start run loop
int
dvap_start(device_t* ctx)
{
  if (!ctx) return FALSE;

  if (!set_run_state(ctx, DVAP_RUN_STATE_RUN)) {
    fprintf(stderr, "Error setting DVAP run state to RUN\n");
    return FALSE;
  }

  pthread_create(&(ctx->watchdog_thread), NULL, watchdog_loop, ctx);
  return TRUE;
}

// Block until dvap_stop() is called then cleanly shutdown
void
dvap_wait(device_t* ctx)
{
  if (!ctx) return;
  pthread_join(ctx->rx_thread, NULL);
  close(ctx->fd);

  pthread_mutex_destroy(&(ctx->shutdown_mutex));
}

// Shutdown DVAP device and shut down threads
int
dvap_stop(device_t* ctx)
{
  if (!ctx) return FALSE;

  if (!set_run_state(ctx, DVAP_RUN_STATE_STOP)) {
    fprintf(stderr, "Error setting DVAP run state to STOP\n");
    return FALSE;
  }
  pthread_mutex_lock(&(ctx->shutdown_mutex));
  ctx->shutdown = TRUE;
  pthread_mutex_unlock(&(ctx->shutdown_mutex));

  return TRUE;
}

int
dvap_write(device_t* ctx, char msg_type, int command, unsigned char* payload, 
           int payload_bytes)
{
  int n;
  int total_sent_bytes = 0;
  int payload_sent_bytes = 0;
  unsigned char buf[4];
  int pktlen;

  if (!ctx) return -1;

  if (payload_bytes > DVAP_MSG_MAX_BYTES-4) {
    fprintf(stderr, "Error in send_command, payload too long\n");
    return -1;
  }

  pktlen = 4 + payload_bytes;
  buf[0] = pktlen & 0xFF;
  buf[1] = (msg_type << 5) | ((pktlen & 0x1F00) >> 8);

  buf[2] = command & 0xFF;
  buf[3] = (command >> 8) & 0xFF;

  if (DEBUG) {
    hex_dump("tx", buf, 4);
  }

  while(total_sent_bytes < 4) {
    pthread_mutex_lock(&(ctx->tx_mutex));
    n = write(ctx->fd, &buf[total_sent_bytes], 4-total_sent_bytes);
    pthread_mutex_unlock(&(ctx->tx_mutex));
    if (n <= 0) {
      debug_print("dvap_write - returned %d\n", n);
      return n;
    }
    total_sent_bytes += n;
  }

  if (payload) {
    if (DEBUG) {
      hex_dump("tx payload", payload, payload_bytes);
    }
    while(payload_sent_bytes < payload_bytes) {
      pthread_mutex_lock(&(ctx->tx_mutex));
      n = write(ctx->fd, &payload[payload_sent_bytes],
                payload_bytes-payload_sent_bytes);
      pthread_mutex_unlock(&(ctx->tx_mutex));
      if (n <= 0) {
        debug_print("dvap_write - returned %d\n", n);
        return n;
      }
      payload_sent_bytes += n;
      total_sent_bytes += n;
    }
  }
  return total_sent_bytes;
}

int
dvap_read(device_t* ctx, char* msg_type, unsigned char* buf, int buf_bytes)
{
  int expected_bytes = 2;
  int received_bytes = 0;
  int n;

  if (!ctx) return -1;

  if (buf_bytes < expected_bytes) {
    fprintf(stderr, "dvap_read - buf_bytes too small\n");
    return -1;
  }

  // Receive header
  n = read(ctx->fd, &buf[0], expected_bytes);
  if (n <= 0) {
    debug_print("dvap_read - returned %d\n", n);
    return n;
  }
  received_bytes += n;

  // Determine how much data to expect
  expected_bytes = buf[0] + ((buf[1] & 0x1F) << 8);
  if (expected_bytes >= buf_bytes) {
    fprintf(stderr, "dvap_read - expected %d bytes but only %d bytes available\n", expected_bytes, buf_bytes);
    return -1;
  }

  // Receive rest of packet
  while (received_bytes < expected_bytes) {
    n = read(ctx->fd, &buf[received_bytes], expected_bytes-received_bytes);
    if (n <= 0) {
      debug_print("dvap_read - returned %d\n", n);
      return n;
    }
    received_bytes += n;
  }

  if (msg_type) {
    *msg_type = (buf[1] & 0xE0) >> 5;
  }

  return received_bytes;
}

int
should_shutdown(device_t* ctx)
{
  int shutdown;
  if (!ctx) return TRUE;
  
  pthread_mutex_lock(&(ctx->shutdown_mutex));
  shutdown = ctx->shutdown;
  pthread_mutex_unlock(&(ctx->shutdown_mutex));
  return shutdown;
}

void*
watchdog_loop(void* arg)
{
  device_t* ctx = (device_t *)arg;
  unsigned char buf[3];

  buf[0] = 0x03;
  buf[1] = 0x60;
  buf[2] = 0x00;

  while (!should_shutdown(ctx)) {
    pthread_mutex_lock(&(ctx->tx_mutex));
    if (DEBUG) {
      hex_dump("tx", buf, 3);
    }
    write(ctx->fd, buf, 3);
    pthread_mutex_unlock(&(ctx->tx_mutex));
    sleep(3);
  }

  return NULL;
}

int
tx_data(device_t* ctx, unsigned char* data, int data_len)
{
  int n;
  int sent_bytes = 0;

  while (sent_bytes < data_len) {
    pthread_mutex_lock(&(ctx->tx_mutex));
    n = write(ctx->fd, &data[sent_bytes], data_len-sent_bytes);
    pthread_mutex_unlock(&(ctx->tx_mutex));
    if (n <= 0) {
      debug_print("tx_data - returned %d\n", n);
      return n;
    }
    sent_bytes += n;
  }

  return sent_bytes;
}

void* 
read_loop(void* arg)
{
  device_t* ctx = (device_t *)arg;

  fd_set set;
  char msg_type;
  int ret;
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  while(!should_shutdown(ctx)) {
    FD_ZERO(&set);
    FD_SET(ctx->fd, &set);
    ret = select(ctx->fd+1, &set, NULL, NULL, &timeout);
    if (ret < 0) {
      fprintf(stderr, "Error waiting for data from DVAP\n");
      return NULL;
    }
    else if (ret == 0) {
      debug_print("%s\n", "DVAP select timeout");
      continue;
    }

    ret = dvap_read(ctx, &msg_type, buf, DVAP_MSG_MAX_BYTES);
    if (ret < 0) {
      fprintf(stderr, "Error reading from DVAP\n");
      return NULL;
    }
    else if (ret == 0) {
      debug_print("%s\n", "DVAP read timeout");
      continue;
    }

    switch (msg_type) {
    case DVAP_MSG_TARGET_ITEM_RESPONSE:
    case DVAP_MSG_TARGET_RANGE_RESPONSE:
      queue_insert(&(ctx->rxq), &buf[2], ret-2);
      if (DEBUG) {
        hex_dump("rx", buf, ret);
      }
      break;
    case DVAP_MSG_TARGET_UNSOLICITED:
      parse_rx_unsolicited(&buf[2], ret-2);
      break;
    case DVAP_MSG_TARGET_DATA_ACK:
      break;
    case DVAP_MSG_TARGET_DATA_ITEM_0:
    case DVAP_MSG_TARGET_DATA_ITEM_1:
    case DVAP_MSG_TARGET_DATA_ITEM_2:
    case DVAP_MSG_TARGET_DATA_ITEM_3:
      parse_rx_data(buf, ret);
      break;
    default:
      fprintf(stderr, "rx: unrecognized response type: %d\n", msg_type);
      break;
    }
  }

  return NULL;
}

void
parse_rx_unsolicited(unsigned char* buf, int buf_len)
{
  int ctrl_code = (buf[1] << 8) + buf[0];
  switch (ctrl_code) {
  case DVAP_CTRL_OPERATIONAL_STATUS:
    //print_operational_status(buf, buf_len);
    break;
  case DVAP_CTRL_PTT_STATE:
    print_ptt_state(buf, buf_len);
    break;
  case DVAP_CTRL_DTMF_MSG:
    print_dtmf(buf, buf_len);
    break;
  default:
    if (DEBUG) {
      hex_dump("rx", buf, buf_len);
    }
    printf("rx: unsolicited other\n");
    break;
  }
}

void
parse_rx_data(unsigned char* buf, int buf_len)
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
    hex_dump("gmsk header", buf, buf_len);
    break;
  // GMSK data
  case 0xC012:
    if (buf_len < 4) return;
    hex_dump("gmsk data", buf, 4);
    break;
  default:
    fprintf(stderr, "parse_rx_data: unrecognized data\n");
    break;
  }
}

void
print_operational_status(unsigned char* buf, int buf_len)
{
  char rssi, squelch, fifo_free;
  if (buf_len < 5) return;

  rssi = buf[2];
  squelch = buf[3];
  fifo_free = buf[4];

  printf("rssi: %04d, squelch: %d, tx fifo free: %03d\n", rssi, squelch,
         fifo_free);
}

void
print_ptt_state(unsigned char* buf, int buf_len)
{
  if (buf_len < 3) return;
  if (buf[2] <= 0) {
    printf("ptt: receive active\n");
  }
  else {
    printf("ptt: transmit active\n");
  }
}

void
print_dtmf(unsigned char* buf, int buf_len)
{
  if (buf_len < 3) return;
  if (buf[2] == 0) {
    printf("dtmf: release\n");
  }
  else {
    printf("dtmf: %c\n", buf[2]);
  }
}
