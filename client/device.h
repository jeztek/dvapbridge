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
  char msg_type;
  char *msg;
  int msg_bytes;
} rx_message;

int get_name(int fd, char* name, int name_len);
int dvap_write(int fd, char msg_type, int command, char* payload, 
                 int payload_bytes);
int dvap_read(int fd, char* msg_type, char* buf, int buf_bytes);

