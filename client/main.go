package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"
)
import dvap "dev.jeztek.com/dvapbridge/client/lib"

			// TODO: handle DVAP operational status 0x0090
			// TODO: handle VAP PTT State 0x0118
			// TODO: handle DTMF 0x0406

var msgLoopEnabled = true

func msgHandler(device dvap.Config) {
	for msgLoopEnabled {
		select {
		case msg := <-device.MsgChannel:
			fmt.Printf("Received unsolicited message: %d\n", msg.Msgtype)
		case <-time.After(time.Second *3):
			fmt.Printf("Waiting for messages\n")
		}
	}
}

func test(device dvap.Config) {
	fmt.Printf("%s\n", device.GetName())
	fmt.Printf("%s\n", device.GetSerial())
	fmt.Printf("%f\n", device.GetInterfaceVersion())

	bv, fv := device.GetFirmwareVersions()
	fmt.Printf("boot code: %f, firmware: %f\n", bv, fv)

	fmt.Printf("status code: %d\n", device.GetStatusCode())
	device.SetRunState(dvap.DVAP_RUN_STATE_STOP)
	device.SetModulationType(dvap.DVAP_MODULATION_GMSK)
	device.SetOperationMode(dvap.DVAP_OPERATION_NORMAL)
	device.SetSquelchThreshold(-100)
	device.SetLEDControlDVAP()
	device.SetLEDControlHost(25, 100, 50)
	device.SetRxFrequency(146520000)
	device.SetTxFrequency(146520000)
	device.SetRxTxFrequency(146520000)

	lower, upper := device.GetTxFrequencyLimits()
	fmt.Printf("freq limit - lower: %d, upper: %d\n", lower, upper)

	device.SetTxPower(10)
	device.SetTxPower(-12)
	fmt.Printf("bandscan: % d\n", device.GetBandScan(206, 11, 145000000))
}

func main() {
	fmt.Printf("Connecting to DVAP\n")
	device := dvap.Config{Device: "/dev/cu.usbserial-A602S3VR"}
	err := device.Open()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("DVAP device connected\n")

	c := make(chan os.Signal, 2)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		for sig := range c {
			log.Printf("Captured %v, exiting...\n", sig)
			msgLoopEnabled = false
			device.Close()
		}
	}()

	//test(device)
	fmt.Printf("Starting message handler\n")
	go msgHandler(device)

	fmt.Printf("Starting loop\n")
	device.SetModulationType(dvap.DVAP_MODULATION_GMSK)
	device.SetRxTxFrequency(145670000)
	device.SetRunState(dvap.DVAP_RUN_STATE_RUN)
	for msgLoopEnabled {
		time.Sleep(10 * time.Second)
		fmt.Printf("Sleep...\n")
	}
}
