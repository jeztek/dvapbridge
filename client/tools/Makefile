CC = gcc

TARGETS = dvap_debug netsink netsrc parsedump
FLAGS = -pthread
INCLUDES = -I/usr/local/include -I..
LIBS = -L/usr/local/lib

all:	$(TARGETS)

dvap_debug: ../common.c ../device.c ../device_gmsk.c dvap_debug.c \
	../queue.c ../serial.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

netsink: ../common.c ../device_gmsk.c netsink.c ../network.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

netsrc: ../common.c netsrc.c ../network.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

parsedump: ../common.c parsedump.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGETS)
