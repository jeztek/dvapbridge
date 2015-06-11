package main

import (
	"encoding/binary"
	"fmt"
	"time"
)

func gmskParseHeader(msg Message) (urcall string, mycall string) {
	packet := msg.data
	if len(packet) < 47 {
		return "", ""
	}
	
	streamId := binary.LittleEndian.Uint16(packet[2:4])
	framePos := packet[4] >> 3
	seq := packet[5]
	// flags1 := packet[6]
	// flags2 := packet[7]
	// flags3 := packet[8]
	rpt1 := string(packet[9:17])
	rpt2 := string(packet[17:25])

	urcall = string(packet[25:33])
	mycall = string(packet[33:41])

	fmt.Printf("[%d] HEADER:\n", time.Now().Unix())
	fmt.Printf("    client: %s, streamId: %d, framePos: %d, seq: %d\n",
		msg.sender, streamId, framePos, seq)
	fmt.Printf("    rpt1: [%s], rpt2: [%s], urcall: [%s], mycall: [%s]\n",
		rpt1, rpt2, urcall, mycall)

	return urcall, mycall
}

func gmskParseData(msg Message) {
	packet := msg.data
	if len(packet) < 18 {
		return
	}

	streamId := binary.LittleEndian.Uint16(packet[2:4])
	framePos := packet[4] >> 3
	seq := packet[5]

	fmt.Printf("[%d] DATA: client: %s, streamId: %d, framePos: %d, seq: %d\n",
		time.Now().Unix(), msg.sender, streamId, framePos, seq)
}
