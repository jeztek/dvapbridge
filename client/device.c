#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>
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

int
set_tx_power(device_t* ctx, int dbm)
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
dvap_init(device_t* ctx, char* portname, dvap_rx_fptr callback)
{
  int fd;
  if (!ctx) return FALSE;

  ctx->callback = callback;

  // serial port
  fd = serial_open(portname, DVAP_BAUD);
  if (fd < 0) {
    return FALSE;
  }
  ctx->fd = fd;

  // shutdown variables
  ctx->shutdown = FALSE;
  pthread_mutex_init(&(ctx->shutdown_mutex), NULL);

  // transmitter variables
  pthread_mutex_init(&(ctx->tx_mutex), NULL);
  pthread_mutex_init(&(ctx->ptt_mutex), NULL);
  ctx->ptt_active = FALSE;

  // receive queue
  queue_init(&(ctx->rxq));
  pthread_create(&(ctx->rx_thread), NULL, dvap_read_loop, ctx);

  return TRUE;
}

int
dvap_start(device_t* ctx)
{
  if (!ctx) return FALSE;

  if (!set_run_state(ctx, DVAP_RUN_STATE_RUN)) {
    fprintf(stderr, "Error setting DVAP run state to RUN\n");
    return FALSE;
  }

  pthread_create(&(ctx->watchdog_thread), NULL, dvap_watchdog_loop, ctx);
  return TRUE;
}

void
dvap_wait(device_t* ctx)
{
  if (!ctx) return;
  pthread_join(ctx->rx_thread, NULL);
  pthread_join(ctx->watchdog_thread, NULL);
  close(ctx->fd);

  pthread_mutex_destroy(&(ctx->shutdown_mutex));
  pthread_mutex_destroy(&(ctx->tx_mutex));
}

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
dvap_pkt_write(device_t* ctx, unsigned char* buf, int buf_bytes)
{
  int n;
  int sent_bytes = 0;

  pthread_mutex_lock(&(ctx->tx_mutex));
  while (sent_bytes < buf_bytes) {
    n = write(ctx->fd, &buf[sent_bytes], buf_bytes-sent_bytes);
    if (DEBUG) {
      printf("[%ld] dvap_pkt_write: %d bytes\n", time(NULL), buf_bytes);
    }
    if (n <= 0) {
      pthread_mutex_unlock(&(ctx->tx_mutex));
      fprintf(stderr, "dvap_pkt_write - error writing to device\n");
      return -1;
    }
    sent_bytes += n;
  }
  pthread_mutex_unlock(&(ctx->tx_mutex));
  return sent_bytes;
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

  pthread_mutex_lock(&(ctx->tx_mutex));
  while(total_sent_bytes < 4) {
    n = write(ctx->fd, &buf[total_sent_bytes], 4-total_sent_bytes);
    if (n <= 0) {
      pthread_mutex_unlock(&(ctx->tx_mutex));
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
      n = write(ctx->fd, &payload[payload_sent_bytes],
                payload_bytes-payload_sent_bytes);
      if (n <= 0) {
        pthread_mutex_unlock(&(ctx->tx_mutex));
        debug_print("dvap_write - returned %d\n", n);
        return n;
      }
      payload_sent_bytes += n;
      total_sent_bytes += n;
    }
  }
  pthread_mutex_unlock(&(ctx->tx_mutex));
  return total_sent_bytes;
}

int
dvap_read(device_t* ctx, char* msg_type, unsigned char* buf, int buf_bytes)
{
  if (!ctx) return -1;
  return packet_read(ctx->fd, msg_type, buf, buf_bytes);
}

int
dvap_should_shutdown(device_t* ctx)
{
  int shutdown;
  if (!ctx) return TRUE;
  
  pthread_mutex_lock(&(ctx->shutdown_mutex));
  shutdown = ctx->shutdown;
  pthread_mutex_unlock(&(ctx->shutdown_mutex));
  return shutdown;
}

void*
dvap_watchdog_loop(void* arg)
{
  device_t* ctx = (device_t *)arg;
  unsigned char buf[3];
  int ptt_active;
  int counter = DVAP_WATCHDOG_SECS;
  buf[0] = 0x03;
  buf[1] = 0x60;
  buf[2] = 0x00;

  while (!dvap_should_shutdown(ctx)) {
    if (counter <= 0) {
      pthread_mutex_lock(&(ctx->ptt_mutex));
      ptt_active = ctx->ptt_active;
      pthread_mutex_unlock(&(ctx->ptt_mutex));

      // Only send watchdog keepalive message if transmitter is not in use
      if (!ptt_active) {
        pthread_mutex_lock(&(ctx->tx_mutex));
        write(ctx->fd, buf, 3);
        pthread_mutex_unlock(&(ctx->tx_mutex));
        counter = DVAP_WATCHDOG_SECS;
        if (DEBUG) {
          hex_dump("watchdog tx", buf, 3);
        }
      }
    }
    counter -= 1;
    sleep(1);
  }

  return NULL;
}

void* 
dvap_read_loop(void* arg)
{
  device_t* ctx = (device_t *)arg;

  fd_set set;
  char msg_type;
  int ret;
  unsigned char buf[DVAP_MSG_MAX_BYTES];

  struct timeval timeout;
  while(!dvap_should_shutdown(ctx)) {
    // Check if data is available for reading
    // Timeout and try again if nothing is available
    FD_ZERO(&set);
    FD_SET(ctx->fd, &set);
    // Linux version of select overwrites timeout, so we set it on each
    // iteration of the while loop
    timeout.tv_sec = 0;
    timeout.tv_usec = DVAP_READ_TIMEOUT_USEC;
    ret = select(ctx->fd+1, &set, NULL, NULL, &timeout);
    if (ret < 0) {
      fprintf(stderr, "Error waiting for data from DVAP\n");
      return NULL;
    }
    else if (ret == 0) {
      //debug_print("%s\n", "DVAP select timeout");
      continue;
    }

    // Read packet
    ret = dvap_read(ctx, &msg_type, buf, DVAP_MSG_MAX_BYTES);
    if (ret < 0) {
      fprintf(stderr, "Error reading from DVAP\n");
      return NULL;
    }
    else if (ret == 0) {
      fprintf(stderr, "Timeout while reading from DVAP\n");
      return NULL;
    }

    // Call appropriate handler depending on message type
    switch (msg_type) {

    // Response to a host initiated request
    case DVAP_MSG_TARGET_ITEM_RESPONSE:
    case DVAP_MSG_TARGET_RANGE_RESPONSE:
      queue_insert(&(ctx->rxq), &buf[2], ret-2);
      if (DEBUG) {
        hex_dump("rx", buf, ret);
      }
      break;

    // Status message
    case DVAP_MSG_TARGET_UNSOLICITED:
      dvap_parse_rx_unsolicited(ctx, &buf[2], ret-2);
      break;

    // Ignore target data acks for now
    case DVAP_MSG_TARGET_DATA_ACK:
      break;

    // Radio data
    case DVAP_MSG_TARGET_DATA_ITEM_0:
    case DVAP_MSG_TARGET_DATA_ITEM_1:
    case DVAP_MSG_TARGET_DATA_ITEM_2:
    case DVAP_MSG_TARGET_DATA_ITEM_3:
      (ctx->callback)(buf, ret);
      break;

    default:
      fprintf(stderr, "rx: unrecognized response type: %d\n", msg_type);
      break;
    }
  }

  return NULL;
}

void
dvap_parse_rx_unsolicited(device_t* ctx, unsigned char* buf, int buf_len)
{
  int ctrl_code = (buf[1] << 8) + buf[0];

  switch (ctrl_code) {
  case DVAP_CTRL_OPERATIONAL_STATUS:
    //dvap_print_operational_status(buf, buf_len);
    break;
  case DVAP_CTRL_PTT_STATE:
    //dvap_print_ptt_state(buf, buf_len);
    pthread_mutex_lock(&(ctx->ptt_mutex));
    if (buf_len >= 3) {
      if (buf[2] <= 0) {
        ctx->ptt_active = FALSE;
      }
      else {
        ctx->ptt_active = TRUE;
      }
    }
    pthread_mutex_unlock(&(ctx->ptt_mutex));
    break;
  case DVAP_CTRL_DTMF_MSG:
    dvap_print_dtmf(buf, buf_len);
    break;
  default:
    if (DEBUG) {
      hex_dump("rx unsolicited other", buf, buf_len);
    }
    break;
  }
}

void
dvap_print_operational_status(unsigned char* buf, int buf_len)
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
dvap_print_ptt_state(unsigned char* buf, int buf_len)
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
dvap_print_dtmf(unsigned char* buf, int buf_len)
{
  if (buf_len < 3) return;
  if (buf[2] == 0) {
    printf("dtmf: release\n");
  }
  else {
    printf("dtmf: %c\n", buf[2]);
  }
}
