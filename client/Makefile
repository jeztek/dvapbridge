CC = gcc

TARGET = client
FLAGS = -pthread
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib

all: $(TARGET)

$(TARGET): common.c serial.c device.c main.c queue.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

qtest: qtest.c queue.c
	$(CC) -Wall -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGET)
