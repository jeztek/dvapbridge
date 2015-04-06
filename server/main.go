// main.go
// DVAP bridge server

package main

import (
	"os"
	"github.com/alecthomas/kingpin"
)

var (
	app = kingpin.New("server", "DVAP Bridge Server")
	debug = app.Flag("debug", "Enable debug mode").Short('d').Bool()
	logfile = app.Flag("logfile", "Log data to file").Short('l').String()
)

func main() {
	kingpin.MustParse(app.Parse(os.Args[1:]))
	server := NewServer()

	if (*logfile != "") {
		server.SetLogfile(*logfile)
	}

	server.Start()
}

