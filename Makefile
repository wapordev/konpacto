CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include/SDL2
LDFLAGS = -L/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/lib -lSDL2

TARGET = main
SOURCES = main.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean