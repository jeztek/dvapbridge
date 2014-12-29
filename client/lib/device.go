package dvapbridge

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"time"
)
import goserial "github.com/tarm/goserial"

const DVAP_BAUD = 230400

const DVAP_MSG_HOST_SET_CTRL       = 0x00
const DVAP_MSG_HOST_REQ_CTRL_ITEM  = 0x01
const DVAP_MSG_HOST_REQ_CTRL_RANGE = 0x02
const DVAP_MSG_HOST_DATA_ACK       = 0x03
const DVAP_MSG_HOST_DATA_ITEM_0    = 0x04
const DVAP_MSG_HOST_DATA_ITEM_1    = 0x05
const DVAP_MSG_HOST_DATA_ITEM_2    = 0x06
const DVAP_MSG_HOST_DATA_ITEM_3    = 0x07

const DVAP_MSG_TARGET_ITEM_RESPONSE  = 0x00
const DVAP_MSG_TARGET_UNSOLICITED    = 0x01
const DVAP_MSG_TARGET_RANGE_RESPONSE = 0x02
const DVAP_MSG_TARGET_DATA_ACK       = 0x03
const DVAP_MSG_TARGET_DATA_ITEM_0    = 0x04
const DVAP_MSG_TARGET_DATA_ITEM_1    = 0x05
const DVAP_MSG_TARGET_DATA_ITEM_2    = 0x06
const DVAP_MSG_TARGET_DATA_ITEM_3    = 0x07

const DVAP_CTRL_TARGET_NAME        = 0x0001
const DVAP_CTRL_TARGET_SERIAL      = 0x0002
const DVAP_CTRL_IFACE_VER          = 0x0003
const DVAP_CTRL_HW_VER             = 0x0004
const DVAP_CTRL_STATUS             = 0x0005
const DVAP_CTRL_RUN_STATE          = 0x0018
const DVAP_CTRL_MODULATION_TYPE    = 0x0028
const DVAP_CTRL_OPERATION_MODE     = 0x002A
const DVAP_CTRL_SQUELCH_THRESH     = 0x0080
const DVAP_CTRL_OPERATIONAL_STATUS = 0x0090
const DVAP_CTRL_PTT_STATE          = 0x0118
const DVAP_CTRL_EXTERNAL_TR_CTRL   = 0x011A
const DVAP_CTRL_LED_CONTROL        = 0x011C
const DVAP_CTRL_RX_FREQ            = 0x0020
const DVAP_CTRL_TX_FREQ            = 0x0120
const DVAP_CTRL_TX_RX_FREQ         = 0x0220
const DVAP_CTRL_READ_TX_FREQ_LIM   = 0x0230
const DVAP_CTRL_TX_POWER           = 0x0138
const DVAP_CTRL_FREQ_CALIB         = 0x0400
const DVAP_CTRL_BAND_SCAN          = 0x0404
const DVAP_CTRL_DTMF_MSG           = 0x0406

const DVAP_STATUS_STOPPED          = 0x00
const DVAP_STATUS_RUNNING          = 0x01
const DVAP_STATUS_BOOT_IDLE        = 0x0E
const DVAP_STATUS_BOOT_PROG        = 0x0F
const DVAP_STATUS_BOOT_PROG_ERR    = 0x80

const DVAP_RUN_STATE_STOP          = 0x00
const DVAP_RUN_STATE_RUN           = 0x01

const DVAP_MODULATION_FM           = 0x00
const DVAP_MODULATION_GMSK         = 0x01

const DVAP_OPERATION_NORMAL        = 0x00
const DVAP_OPERATION_CWTEST        = 0x01
const DVAP_OPERATION_DEVIATIONTEST = 0x02

const DVAP_PTT_RX_ACTIVE           = 0x00
const DVAP_PTT_TX_ACTIVE           = 0x01

const DVAP_DATA_FM_HDR             = 0x4281
const DVAP_DATA_GMSK_HDR           = 0x2FA0
const DVAP_GMSK_TX_ACK_HDR         = 0x2F60

const DVAP_MSG_MAX_BYTES           = 8191

type RxMessage struct {
	msgtype byte
	msg     []byte
}

type Config struct {
	Device string
	serial io.ReadWriteCloser

	txLoopEnabled bool
	txChannel     chan []byte

	rxLoopEnabled bool
	rxChannel     chan RxMessage
}

func (c *Config) Open() error {
	var err error
	s := &goserial.Config{Name: c.Device, Baud: DVAP_BAUD}
	c.serial, err = goserial.OpenPort(s)

	if err == nil {
		c.txLoopEnabled = true
		c.txChannel = make(chan []byte)
		go c.serialTxLoop()

		c.rxLoopEnabled = true
		c.rxChannel = make(chan RxMessage)
		go c.serialRxLoop()
	}

	return err
}

func (c *Config) Close() error {
	c.txLoopEnabled = false
	c.rxLoopEnabled = false
	return c.serial.Close()
}

func (c *Config) GetName() string {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_TARGET_NAME, []byte{})
	rx := <-c.rxChannel
	return string(rx.msg[2:])
}

func (c *Config) GetSerial() string {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_TARGET_SERIAL, []byte{})
	rx := <-c.rxChannel
	return string(rx.msg[2:])
}

func (c *Config) GetInterfaceVersion() float32 {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_IFACE_VER, []byte{})
	rx := <-c.rxChannel
	return float32(binary.LittleEndian.Uint16(rx.msg[2:])) / 100.0
}

func (c *Config) GetFirmwareVersions() (float32, float32) {
	var bootcodeVer, firmwareVer float32

	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_HW_VER, []byte{0x00})
	rx := <-c.rxChannel
	bootcodeVer = float32(binary.LittleEndian.Uint16(rx.msg[3:])) / 100.0

	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_HW_VER, []byte{0x01})
	rx = <-c.rxChannel
	firmwareVer = float32(binary.LittleEndian.Uint16(rx.msg[3:])) / 100.0

	return bootcodeVer, firmwareVer
}

func (c *Config) GetStatusCode() byte {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_STATUS, []byte{})
	rx := <-c.rxChannel
	return rx.msg[2]
}

func (c *Config) SetRunState(state byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_RUN_STATE, []byte{state})
	<-c.rxChannel
}

func (c *Config) SetModulationType(modulation byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_MODULATION_TYPE, []byte{modulation})
	<-c.rxChannel
}

func (c *Config) SetOperationMode(mode byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_OPERATION_MODE, []byte{mode})
	<-c.rxChannel
}

func (c *Config) serialTx(msg []byte) {
	msgBytes := len(msg)
	var sentBytes int = 0

	// Loop until all data is sent
	for sentBytes < msgBytes {
		n, err := c.serial.Write(msg[sentBytes:msgBytes])
		if n == 0 || err != nil {
			log.Panic(err)
			break
		}
		sentBytes += n
	}
	fmt.Printf("tx: % X\n", msg)
}

func (c *Config) serialTxLoop() {
	for c.txLoopEnabled {
		select {
		case msg := <-c.txChannel:
			c.serialTx(msg)
		case <-time.After(time.Second * 3):
			c.serialTx([]byte{0x03, 0x60, 0x00})
		}
	}
}

func (c *Config) serialRxLoop() {
	buf := make([]byte, DVAP_MSG_MAX_BYTES)

	for c.rxLoopEnabled {
		var receivedBytes int = 0
		var expectedBytes int = 2

		// Receive header
		for receivedBytes < expectedBytes {
			n, err := c.serial.Read(buf[receivedBytes:])
			if n == 0 || err != nil {
				log.Fatal(err)
			}
			receivedBytes += n
		}

		// Receive rest of packet
		expectedBytes = int(buf[0]) + int((buf[1] & 0x1F) << 8)
		for receivedBytes < expectedBytes {
			n, err := c.serial.Read(buf[receivedBytes:expectedBytes])
			if n == 0 || err != nil {
				log.Fatal(err)
			}
			receivedBytes += n
		}

		// Send to channel
		fmt.Printf("rx: % X\n", buf[:expectedBytes])
		var response RxMessage
		response.msgtype = byte((buf[1] & 0xE0) >> 5)
		response.msg = buf[2:expectedBytes]
		c.rxChannel <- response
	}
}

func (c *Config) sendControlCommand(msgtype byte, command uint16, payload []byte) {
	if len(payload) > DVAP_MSG_MAX_BYTES-4 {
		log.Fatal("Payload too long")
	}

	pkt := new(bytes.Buffer)

	// Write dummy header
	header := []byte{0x00, 0x00}
	_, err := pkt.Write(header)
	if err != nil {
		log.Fatal(err)
	}

	// Write command
	err = binary.Write(pkt, binary.LittleEndian, command)
	if err != nil {
		log.Fatal(err)
	}

	// Write payload
	for _, v := range payload {
		err = binary.Write(pkt, binary.LittleEndian, v)
		if err != nil {
			log.Fatal(err)
		}
	}

	// Update header
	var pktlen int = 4 + len(payload)
	pkt.Bytes()[0] = byte(pktlen & 0xFF)
	pkt.Bytes()[1] = (msgtype << 5) | byte((pktlen&0x1F00)>>8)

	c.txChannel <- pkt.Bytes()
}

/*
Control message [16 bit header | 16 bit control item | payload]
Data item message [16 bit header | data bytes]
header [8 bit length lsb | 3 bit type | 5 bit length msb]
*/
