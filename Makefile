CC = gcc
CFLAGS = -Wall -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include/SDL -I/usr/include -DSDL_DISABLE_IMMINTRIN_H
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -L/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/lib `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

TARGET = main
SOURCES = main.c

all: run

konpacto:
	$(CC) src/screen.c src/input.c src/pages.c src/file.c src/ui.c src/main.c -I../SDL2-2.30.9/x86_64-w64-mingw32/include/SDL2 -I../SDL2_image-2.8.2/x86_64-w64-mingw32/include/SDL2 -I../SDL2_mixer-2.8.0/x86_64-w64-mingw32/include/SDL2 -L../SDL2-2.30.9/x86_64-w64-mingw32/lib -L../SDL2_image-2.8.2/x86_64-w64-mingw32/lib -L../SDL2_mixer-2.8.0/x86_64-w64-mingw32/lib -lSDL2 -lSDL2_mixer -lSDL2_image -o build/main

run: konpacto
	cd build && main.exe

.PHONY: all run konpacto

