package dvapbridge

// #include <fcntl.h>
// #include <sys/ioctl.h>
// #include <termios.h>
// #include <unistd.h>
import "C"

import (
	"errors"
	"fmt"
	"io"
	"os"
	"syscall"
	"time"
)

var bauds = map[int]C.speed_t{
	2400: C.B2400,
	4800: C.B4800,
	9600: C.B9600,
	19200: C.B19200,
	38400: C.B38400,
	57600: C.B57600,
	115200: C.B115200,
	230400: C.B230400,
}

func openPort(name string, baud int, readTimeout time.Duration) (rwc io.ReadWriteCloser, err error) {
	file, err := os.OpenFile(name, syscall.O_RDWR|syscall.O_NOCTTY|syscall.O_NONBLOCK, 0666)
	if err != nil {
		return nil, err
	}

	defer func() {
		if err != nil {
			fmt.Println("DEBUG: Error in openPort(), closing port")
			file.Close()
		}
	}()

	fd := C.int(file.Fd())

	if C.isatty(fd) != 1 {
		err = errors.New("File is not a tty")
		return nil, err
	}

	// Get exclusive access to port
	r0, _, errno := syscall.Syscall(syscall.SYS_IOCTL, uintptr(fd), C.TIOCEXCL, 0)
	if r0 != 0 {
		err = fmt.Errorf("Error setting TIOCEXCL: %s", errno)
		return nil, err
	}

	// Clear O_NONBLOCK flag
	r0, _, errno = syscall.Syscall(syscall.SYS_FCNTL, uintptr(fd), C.F_SETFL, 0)
	if r0 != 0 {
		err = fmt.Errorf("Error clearing O_NONBLOCK: %s", errno)
		return nil, err
	}

	// Get current options
	var r C.int
	var termiosOrig C.struct_termios
	r, err = C.tcgetattr(fd, &termiosOrig)
	if r != 0 {
		err = fmt.Errorf("Error getting termios: %s", err)
		return nil, err
	}
// 	defer func() {
// 		C.tcsetattr(fd, C.TCSANOW, &termiosOrig)
// 	}()

	termios := termiosOrig

	// Raw mode
	C.cfmakeraw(&termios)

	// Local mode
	termios.c_cflag |= (C.CLOCAL | C.CREAD)

	// Timeout (no inter-character timeout but set overall timeout)
	// http://www.unixwiz.net/techtips/termios-vmin-vtime.html
	termios.c_cc[C.VMIN] = 0
	timeout := readTimeout / (time.Second / 10)
	fmt.Printf("Set read timeout to %d\n", int(timeout))
	termios.c_cc[C.VTIME] = C.cc_t(timeout)

	speed, ok := bauds[baud]
	if !ok {
		err = fmt.Errorf("Unknown baud rate %d", baud)
		return nil, err
	}

	r, err = C.cfsetispeed(&termios, speed)
	if r != 0 {
		err = fmt.Errorf("Error setting input speed: %d (%s)", speed, err)
		return nil, err
	}

	r, err = C.cfsetospeed(&termios, speed)
	if r != 0 {
		err = fmt.Errorf("Error setting output speed: %d (%s)", speed, err)
		return nil, err
	}

	// 8-N-1
	termios.c_cflag |= C.CS8
	termios.c_cflag &^= C.PARENB
	termios.c_cflag &^= C.CSTOPB

	r, err = C.tcsetattr(fd, C.TCSAFLUSH, &termios)
	if r != 0 {
		err = fmt.Errorf("Error setting termios: %s", err)
		return nil, err
	}

	return file, nil
}
