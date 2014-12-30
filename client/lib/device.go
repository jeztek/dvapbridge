// TODO:
// * Verify success/failure of commands

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

const DVAP_SQUELCH_MIN             = -128
const DVAP_SQUELCH_MAX             = -45

const DVAP_LED_INTENSITY_MIN       = 0
const DVAP_LED_INTENSITY_MAX       = 100

const DVAP_TX_POWER_MIN            = -12
const DVAP_TX_POWER_MAX            = 10

const DVAP_BAND_SCAN_STEPS_MIN     = 1
const DVAP_BAND_SCAN_STEPS_MAX     = 800
const DVAP_BAND_SCAN_STRIDE_MIN    = 1
const DVAP_BAND_SCAN_STRIDE_MAX    = 255
const DVAP_BAND_SCAN_FREQ_MIN      = 144000000
const DVAP_BAND_SCAN_FREQ_MAX      = 148000000

type RxMessage struct {
	Msgtype byte
	Msg     []byte
}

type Config struct {
	Device string
	serial io.ReadWriteCloser

	txLoopEnabled bool
	txChannel     chan []byte

	rxLoopEnabled bool
	rxChannel     chan RxMessage
	
	// Channel for unsolicited messages
	MsgChannel    chan RxMessage
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
	return string(rx.Msg[2:])
}

func (c *Config) GetSerial() string {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_TARGET_SERIAL, []byte{})
	rx := <-c.rxChannel
	return string(rx.Msg[2:])
}

func (c *Config) GetInterfaceVersion() float32 {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_IFACE_VER, []byte{})
	rx := <-c.rxChannel
	return float32(binary.LittleEndian.Uint16(rx.Msg[2:])) / 100.0
}

func (c *Config) GetFirmwareVersions() (bootcodeVer float32, firmwareVer float32) {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_HW_VER, []byte{0x00})
	rx := <-c.rxChannel
	bootcodeVer = float32(binary.LittleEndian.Uint16(rx.Msg[3:])) / 100.0

	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_HW_VER, []byte{0x01})
	rx = <-c.rxChannel
	firmwareVer = float32(binary.LittleEndian.Uint16(rx.Msg[3:])) / 100.0
	return
}

func (c *Config) GetStatusCode() byte {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_STATUS, []byte{})
	rx := <-c.rxChannel
	return rx.Msg[2]
}

func (c *Config) SetRunState(state byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_RUN_STATE, []byte{state})
	<-c.rxChannel
}

// NOTE: Can only be sent when DVAP is stopped
func (c *Config) SetModulationType(modulation byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_MODULATION_TYPE, []byte{modulation})
	<-c.rxChannel
}

func (c *Config) SetOperationMode(mode byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_OPERATION_MODE, []byte{mode})
	<-c.rxChannel
}

func (c *Config) SetSquelchThreshold(dbm int) {
	squelch := dbm
	if dbm < DVAP_SQUELCH_MIN {
		squelch = DVAP_SQUELCH_MIN
	}
	if dbm > DVAP_SQUELCH_MAX {
		squelch = DVAP_SQUELCH_MAX
	}
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_SQUELCH_THRESH, []byte{byte(squelch & 0xFF)})
	<-c.rxChannel
}

func (c *Config) SetExternalTRControlMode(mode byte) {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_EXTERNAL_TR_CTRL, []byte{mode})
	<-c.rxChannel
}

func (c *Config) SetLEDControlDVAP() {
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_LED_CONTROL, []byte{0x00, 0x00, 0x00, 0x00})
	<-c.rxChannel
}

// NOTE: Valid intensity values are 0 to 100 (0 = off, 100 = max)
func (c *Config) SetLEDControlHost(yellow uint, red uint, green uint) {
	var y, r, g byte
	if yellow < DVAP_LED_INTENSITY_MIN {
		yellow = DVAP_LED_INTENSITY_MIN
	}
	if yellow > DVAP_LED_INTENSITY_MAX {
		yellow = DVAP_LED_INTENSITY_MAX
	}
	if red < DVAP_LED_INTENSITY_MIN {
		red = DVAP_LED_INTENSITY_MIN
	}
	if red > DVAP_LED_INTENSITY_MAX {
		red = DVAP_LED_INTENSITY_MAX
	}
	if green < DVAP_LED_INTENSITY_MIN {
		green = DVAP_LED_INTENSITY_MIN
	}
	if green > DVAP_LED_INTENSITY_MAX {
		green = DVAP_LED_INTENSITY_MAX
	}
	y = byte(yellow & 0xFF)
	r = byte(red & 0xFF)
	g = byte(green & 0xFF)
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_LED_CONTROL, []byte{0x01, y, r, g})
	<-c.rxChannel
}

func (c *Config) SetRxFrequency(frequency uint32) {
	freq := make([]byte, 4)
	binary.LittleEndian.PutUint32(freq, frequency)
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_RX_FREQ, freq)
	<-c.rxChannel
}

func (c *Config) SetTxFrequency(frequency uint32) {
	freq := make([]byte, 4)
	binary.LittleEndian.PutUint32(freq, frequency)
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_TX_FREQ, freq)
	<-c.rxChannel
}

func (c *Config) SetRxTxFrequency(frequency uint32) {
	freq := make([]byte, 4)
	binary.LittleEndian.PutUint32(freq, frequency)
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_TX_RX_FREQ, freq)
	<-c.rxChannel
}

func (c *Config) GetTxFrequencyLimits() (lower uint32, upper uint32) {
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_READ_TX_FREQ_LIM, []byte{})
	rx := <-c.rxChannel
	lower = binary.LittleEndian.Uint32(rx.Msg[2:])
	upper = binary.LittleEndian.Uint32(rx.Msg[6:])
	return
}

// Valid range is -12 dBm to 10 dBm
func (c *Config) SetTxPower(dbm int) {
	power := dbm
	if dbm < DVAP_TX_POWER_MIN {
		power = DVAP_TX_POWER_MIN
	}
	if dbm > DVAP_TX_POWER_MAX {
		power = DVAP_TX_POWER_MAX
	}

	pwr := make([]byte, 2)
	binary.LittleEndian.PutUint16(pwr, uint16(power))
	c.sendControlCommand(DVAP_MSG_HOST_SET_CTRL, DVAP_CTRL_TX_POWER, pwr)
	<-c.rxChannel
}

// Valid number of steps is 1 to 800
// Valid step size is 100Hz to 25,500Hz in 100 Hz increments (1, 255)
// Valid start frequency range is 144,000,000 to 148,000,000
func (c *Config) GetBandScan(numSteps uint16, stepSize byte, startFreq uint32) (rssi []byte) {
	if numSteps < DVAP_BAND_SCAN_STEPS_MIN {
		numSteps = DVAP_BAND_SCAN_STEPS_MIN
	}
	if numSteps > DVAP_BAND_SCAN_STEPS_MAX {
		numSteps = DVAP_BAND_SCAN_STEPS_MAX
	}
	if stepSize < DVAP_BAND_SCAN_STRIDE_MIN {
		stepSize = DVAP_BAND_SCAN_STRIDE_MIN
	}
	if stepSize > DVAP_BAND_SCAN_STRIDE_MAX {
		stepSize = DVAP_BAND_SCAN_STRIDE_MAX
	}
	if startFreq < DVAP_BAND_SCAN_FREQ_MIN {
		startFreq = DVAP_BAND_SCAN_FREQ_MIN
	}
	if startFreq > DVAP_BAND_SCAN_FREQ_MAX {
		startFreq = DVAP_BAND_SCAN_FREQ_MAX
	}
	
	payload := new(bytes.Buffer)
	binary.Write(payload, binary.LittleEndian, numSteps)
	binary.Write(payload, binary.LittleEndian, stepSize)
	binary.Write(payload, binary.LittleEndian, startFreq)
	c.sendControlCommand(DVAP_MSG_HOST_REQ_CTRL_ITEM, DVAP_CTRL_BAND_SCAN, payload.Bytes())
	rx := <-c.rxChannel
	rssi = rx.Msg[2:]
	return
}

func (c *Config) ParseDTMF() {

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
		fmt.Printf("rx: % X", buf[:expectedBytes])
		var response RxMessage
		response.Msgtype = byte((buf[1] & 0xE0) >> 5)
		response.Msg = buf[2:expectedBytes]
		fmt.Printf(", type: %d\n", response.Msgtype)

		if response.Msgtype == DVAP_MSG_TARGET_UNSOLICITED {
			c.MsgChannel <- response
		} else {
			c.rxChannel <- response
		}
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
	pkt.Bytes()[1] = (msgtype << 5) | byte((pktlen & 0x1F00) >> 8)

	c.txChannel <- pkt.Bytes()
}

/*
Control message [16 bit header | 16 bit control item | payload]
Data item message [16 bit header | data bytes]
header [8 bit length lsb | 3 bit type | 5 bit length msb]
*/
