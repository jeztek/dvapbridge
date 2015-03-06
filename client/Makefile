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

nettest: common.c nettest.c network.c
	$(CC) -Wall -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGET) qtest nettest
