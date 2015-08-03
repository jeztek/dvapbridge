// main.go
// DVAP bridge server

/* TODO:
 * Enable server restarts without dropping connections
 * Server should properly route streams destined for a specific user
 */
package main

import (
	"fmt"
	"github.com/alecthomas/kingpin"
	"os"
	"time"
)

var (
	app     = kingpin.New("server", "DVAP Bridge Server")
	debug   = app.Flag("debug", "Enable debug mode").Short('d').Bool()
	logfile = app.Flag("logfile", "Log data to file").Short('l').String()
)

func Printf(format string, a ...interface{}) {
	fmt.Printf("[%s] ", time.Now().Format(time.RFC822Z))
	fmt.Printf(format, a...)
}

func main() {
	kingpin.MustParse(app.Parse(os.Args[1:]))
	server := NewServer()

	if *logfile != "" {
		server.SetLogfile(*logfile)
	}

	server.Start()
}
