CC = gcc

TARGETS = client qtest
FLAGS = -Wall -pthread
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib

all: $(TARGETS)

client: common.c device.c device_gmsk.c main.c network.c queue.c serial.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

qtest: qtest.c queue.c
	$(CC) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGETS)
