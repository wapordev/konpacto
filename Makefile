CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include/SDL -I/usr/include -DSDL_DISABLE_IMMINTRIN_H
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -L/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/lib `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

TARGET = main
SOURCES = main.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean