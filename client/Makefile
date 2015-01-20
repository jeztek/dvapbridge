CC = gcc

TARGET = device
FLAGS = -pthread
INCLUDES =
LIBS = 

all: $(TARGET)

$(TARGET): serial.c device.c
	$(CC) $(FLAGS) -Wall -o $@ $^ $(INCLUDES) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGET)
