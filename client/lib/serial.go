package dvapbridge

import "io"
import "time"

type SerialConfig struct {
	Name string
	Baud int
	ReadTimeout time.Duration
}

func OpenSerialPort(c *SerialConfig) (io.ReadWriteCloser, error) {
	return openPort(c.Name, c.Baud, c.ReadTimeout)
}
