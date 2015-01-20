// http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "serial.h"

int 
serial_open(char* portname, int speed)
{
  int fd;
  struct termios tty;

  fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "Error opening port %s\n", portname);
    return -1;
  }

  memset(&tty, 0, sizeof(tty));
  if (tcgetattr(fd, &tty) != 0) {
    fprintf(stderr, "Error getting port attributes\n");
    return -1;
  }
  
  if (ioctl(fd, TIOCEXCL) != 0) {
    fprintf(stderr, "Error setting serial port TIOCEXCL flag\n");
    return -1;
  }
  
  if (fcntl(fd, F_SETFL, 0) != 0) {
    fprintf(stderr, "Error clearing serial port O_NONBLOCK flag\n");
    return -1;
  }

  cfmakeraw(&tty);
  tty.c_cflag |= (CLOCAL | CREAD);	// local mode
  tty.c_cflag |= CS8;			// 8-bit chars
  tty.c_cflag &= ~PARENB;		// no parity
  tty.c_cflag &= ~CSTOPB;		// 1 stop bit
  tty.c_cc[VMIN] = 0;			// no inter-char timeout
  tty.c_cc[VTIME] = 5;			// 0.5 second read timeout

  if (cfsetispeed(&tty, speed) != 0) {
    fprintf(stderr, "Error setting serial port input speed\n");
    return -1;
  }

  if (cfsetospeed(&tty, speed) != 0) {
    fprintf(stderr, "Error setting serial port output speed\n");
    return -1;
  }

  if (tcsetattr(fd, TCSAFLUSH, &tty) != 0) {
    fprintf(stderr, "Error setting port attributes\n");
    return -1;
  }

  return fd;

  /*
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;	// 8-bit chars
  tty.c_iflag &= ~IGNBRK;	// disable break processing
  tty.c_lflag = 0;		// no signaling chars, no echo,
  				// no canonical processing
  tty.c_oflag = 0;		// no remapping, no delays
  tty.c_cc[VMIN] = 0;		// read doesn't block
  tty.c_cc[VTIME] = 5;		// 0.5 second read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);	// shut off xon/xoff
  tty.c_cflag |= (CLOCAL | CREAD);		// ignore modem controls
						// enable reading
  tty.c_cflag &= ~(PARENB | PARODD);		// no parity
  tty.c_cflag &= ~CSTOPB;			// 1 stop bit
  tty.c_cflag &= ~CRTSCTS;			// no RTS, CTS

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("Error setting port attributes\n");
    return -1;
  }
  */
}
