#ifndef DEVICE_H
#define DEVICE_H

#include <pthread.h>
#include "queue.h"

#define DVAP_BAUD 230400

#define DVAP_MSG_HOST_SET_CTRL       0x00
#define DVAP_MSG_HOST_REQ_CTRL_ITEM  0x01
#define DVAP_MSG_HOST_REQ_CTRL_RANGE 0x02
#define DVAP_MSG_HOST_DATA_ACK       0x03
#define DVAP_MSG_HOST_DATA_ITEM_0    0x04
#define DVAP_MSG_HOST_DATA_ITEM_1    0x05
#define DVAP_MSG_HOST_DATA_ITEM_2    0x06
#define DVAP_MSG_HOST_DATA_ITEM_3    0x07

#define DVAP_MSG_TARGET_ITEM_RESPONSE  0x00
#define DVAP_MSG_TARGET_UNSOLICITED    0x01
#define DVAP_MSG_TARGET_RANGE_RESPONSE 0x02
#define DVAP_MSG_TARGET_DATA_ACK       0x03
#define DVAP_MSG_TARGET_DATA_ITEM_0    0x04
#define DVAP_MSG_TARGET_DATA_ITEM_1    0x05
#define DVAP_MSG_TARGET_DATA_ITEM_2    0x06
#define DVAP_MSG_TARGET_DATA_ITEM_3    0x07

#define DVAP_CTRL_TARGET_NAME        0x0001
#define DVAP_CTRL_TARGET_SERIAL      0x0002
#define DVAP_CTRL_IFACE_VER          0x0003
#define DVAP_CTRL_HW_VER             0x0004
#define DVAP_CTRL_STATUS             0x0005
#define DVAP_CTRL_RUN_STATE          0x0018
#define DVAP_CTRL_MODULATION_TYPE    0x0028
#define DVAP_CTRL_OPERATION_MODE     0x002A
#define DVAP_CTRL_SQUELCH_THRESH     0x0080
#define DVAP_CTRL_OPERATIONAL_STATUS 0x0090
#define DVAP_CTRL_PTT_STATE          0x0118
#define DVAP_CTRL_EXTERNAL_TR_CTRL   0x011A
#define DVAP_CTRL_LED_CONTROL        0x011C
#define DVAP_CTRL_RX_FREQ            0x0020
#define DVAP_CTRL_TX_FREQ            0x0120
#define DVAP_CTRL_TX_RX_FREQ         0x0220
#define DVAP_CTRL_READ_TX_FREQ_LIM   0x0230
#define DVAP_CTRL_TX_POWER           0x0138
#define DVAP_CTRL_FREQ_CALIB         0x0400
#define DVAP_CTRL_BAND_SCAN          0x0404
#define DVAP_CTRL_DTMF_MSG           0x0406

#define DVAP_STATUS_STOPPED          0x00
#define DVAP_STATUS_RUNNING          0x01
#define DVAP_STATUS_BOOT_IDLE        0x0E
#define DVAP_STATUS_BOOT_PROG        0x0F
#define DVAP_STATUS_BOOT_PROG_ERR    0x80

#define DVAP_RUN_STATE_STOP          0x00
#define DVAP_RUN_STATE_RUN           0x01

#define DVAP_MODULATION_FM           0x00
#define DVAP_MODULATION_GMSK         0x01

#define DVAP_OPERATION_NORMAL        0x00
#define DVAP_OPERATION_CWTEST        0x01
#define DVAP_OPERATION_DEVIATIONTEST 0x02

#define DVAP_PTT_RX_ACTIVE           0x00
#define DVAP_PTT_TX_ACTIVE           0x01

#define DVAP_DATA_FM_HDR             0x4281
#define DVAP_DATA_GMSK_HDR           0x2FA0
#define DVAP_GMSK_TX_ACK_HDR         0x2F60

#define DVAP_MSG_MAX_BYTES           8191

#define DVAP_SQUELCH_MIN             -128
#define DVAP_SQUELCH_MAX             -45

#define DVAP_LED_INTENSITY_MIN       0
#define DVAP_LED_INTENSITY_MAX       100

#define DVAP_TX_POWER_MIN            -12
#define DVAP_TX_POWER_MAX            10

#define DVAP_BAND_SCAN_STEPS_MIN     1
#define DVAP_BAND_SCAN_STEPS_MAX     800
#define DVAP_BAND_SCAN_STRIDE_MIN    1
#define DVAP_BAND_SCAN_STRIDE_MAX    255
#define DVAP_BAND_SCAN_FREQ_MIN      144000000
#define DVAP_BAND_SCAN_FREQ_MAX      148000000

typedef struct {
  int fd;

  int shutdown;
  pthread_mutex_t shutdown_mutex;

  pthread_mutex_t tx_mutex;
  pthread_t watchdog_thread;

  queue_t rxq;
  pthread_t rx_thread;

} device_t;

typedef struct {
  unsigned char header[2];
  unsigned char audio[320];
} dvap_fm_data_t;

typedef struct {
  unsigned char header[2];
  unsigned char stream_id[2];
  unsigned char frame_pos;
  unsigned char seq;
  unsigned char flags1;
  unsigned char flags2;
  unsigned char flags3;
  unsigned char rpt1[8];
  unsigned char rpt2[8];
  unsigned char urcall[8];
  unsigned char mycall[8];
  unsigned char mysuffix[4];
  unsigned char pfcs[2];
} dvap_dstar_header_t;

typedef struct {
  unsigned char header[2];
  unsigned char stream_id0[2];
  unsigned char stream_id1[2];
  unsigned char seq : 5;
  unsigned char header_flag : 1;
  unsigned char end_of_stream_flag : 1;
  unsigned char using_prev_header_packet_flag : 1;
  unsigned char data[12];
} dvap_dstar_data_t;

typedef struct {
  char msg_type;
  char *msg;
  int msg_bytes;
} rx_message;

int get_name(device_t* ctx, char* name, int name_len);
int get_serial(device_t* ctx, char* serial, int serial_len);
int get_interface_version(device_t* ctx, float* version);
int get_firmware_version(device_t* ctx, float* bootcode, float* firmware);
int get_status_code(device_t* ctx, char* code);
int get_tx_frequency_limits(device_t* ctx, unsigned int* lower,
                            unsigned int* upper);
int get_band_scan(device_t* ctx, unsigned int num_steps, char step_size,
                  unsigned int start_freq_hz);

int set_run_state(device_t* ctx, char state);
int set_modulation_type(device_t* ctx, char modulation);
int set_operation_mode(device_t* ctx, char mode);
int set_squelch_threshold(device_t* ctx, int dbm);
int set_external_tr_control_mode(device_t* ctx, char mode);
int set_led_control_dvap(device_t* ctx);
int set_led_control_host(device_t* ctx, unsigned int yellow, unsigned int red,
                         unsigned int green);
int set_rx_frequency(device_t* ctx, unsigned int hz);
int set_tx_frequency(device_t* ctx, unsigned int hz);
int set_rxtx_frequency(device_t* ctx, unsigned int hz);
int set_tx_power(device_t* ctx, int dbm);

int dvap_init(device_t* ctx);
int dvap_start(device_t* ctx);
void dvap_wait(device_t* ctx);
int dvap_stop(device_t* ctx);
int dvap_write(device_t* ctx, char msg_type, int command,
               unsigned char* payload, int payload_bytes);
int dvap_read(device_t* ctx, char* msg_type, unsigned char* buf,
              int buf_bytes);

int should_shutdown();
void* watchdog_loop(void* arg);
void* read_loop(void* arg);
void parse_rx_unsolicited(unsigned char* buf, int buf_len);
void print_operational_status(unsigned char* buf, int buf_len);

#endif
