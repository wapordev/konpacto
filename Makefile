CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include/SDL
LDFLAGS = -L/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/lib -lSDL

TARGET = hello-world
SOURCES = hello-world.c

all: $(TARGET)

$(TARGET): $(SOURCES)
    $(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
    rm -f $(TARGET)

.PHONY: all clean