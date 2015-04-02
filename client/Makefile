CC = gcc

TARGET = client
FLAGS = -pthread
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib

all: $(TARGET)

$(TARGET): common.c device.c device_gmsk.c main.c network.c queue.c serial.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

qtest: qtest.c queue.c
	$(CC) -Wall -o $@ $^

netsink: common.c device_gmsk.c netsink.c network.c
	$(CC) $(FLAGS) -Wall -o $@ $^

netsrc: common.c netsrc.c network.c
	$(CC) $(FLAGS) -Wall -o $@ $^

nettest: common.c nettest.c network.c
	$(CC) $(FLAGS) -Wall -o $@ $^

parsedump: common.c parsedump.c
	$(CC) $(FLAGS) -Wall -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGET) qtest nettest netsink netsrc parsedump
