package main

import (
	"fmt"
	"log"
)
import dvap "dev.jeztek.com/dvapbridge/client/lib"

func main() {
	dvap := dvap.Config{Device: "/dev/cu.usbserial-A602S3VR"}
	err := dvap.Open()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s\n", dvap.GetName())
	fmt.Printf("%s\n", dvap.GetSerial())
	fmt.Printf("%f\n", dvap.GetInterfaceVersion())

	bv, fv := dvap.GetFirmwareVersions()
	fmt.Printf("boot code: %f, firmware: %f\n", bv, fv)

	fmt.Printf("status code: %d\n", dvap.GetStatusCode())
}
