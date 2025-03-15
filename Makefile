CC = gcc
CFLAGS = -Wall -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include/SDL -I/usr/include -DSDL_DISABLE_IMMINTRIN_H
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -L/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/lib `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

TARGET = main
SOURCES = main.c




KONPACTO_FILES = src/lua.c src/screen.c src/input.c src/pages.c src/file.c src/ui.c src/main.c src/sound.c src/synth.c src/sequence.c

KONPACTO_OBJECTS = lua.o screen.o input.o pages.o file.o ui.o main.o sound.o synth.o sequence.o

KONPACTO_INCLUDES = -I./tracy/public/tracy -I../lua/5.1.5/include -I../tinydir-1.2.6 -I../SDL2-2.30.9/x86_64-w64-mingw32/include/SDL2 -I../SDL2_image-2.8.2/x86_64-w64-mingw32/include/SDL2 -I../SDL2_mixer-2.8.0/x86_64-w64-mingw32/include/SDL2 -I../portaudio

KONPACTO_LINKS = -L../luajit/src -L../SDL2-2.30.9/x86_64-w64-mingw32/lib -L../SDL2_image-2.8.2/x86_64-w64-mingw32/lib -L../SDL2_mixer-2.8.0/x86_64-w64-mingw32/lib -L../portaudio

KONPACTO_LIBRARIES = -lluajit-5.1 -lSDL2 -lSDL2_mixer -lSDL2_image -lportaudio

KONPACTO_PROF = -DTRACY_ENABLE

KONPACTO_HEADLESS = -Wl,--subsystem,windows

KONPACTO_RELEASE = -Ofast

all: run

konpacto:
	gcc -march=native -DTRACY_IMPORTS -DTRACY_ENABLE -c -std=c99 $(KONPACTO_FILES) $(KONPACTO_INCLUDES) $(KONPACTO_LINKS) $(KONPACTO_LIBRARIES) $(KONPACTO_RELEASE) $(KONPACTO_PROF)
	g++ -march=native -DTRACY_EXPORTS -DTRACY_ENABLE -c -std=c++11 -o build/tracy.o -Itracy/public/tracy tracy/public/TracyClient.cpp
	g++ -march=native -std=c++11 $(KONPACTO_LINKS) -DTRACY_ENABLE -o build/main build/tracy.o -lws2_32 -lwsock32 -limagehlp -ldbghelp -lstdc++ -pthread $(KONPACTO_OBJECTS) $(KONPACTO_LIBRARIES)


luatest: lua
	cd build && test.exe

lua:
	$(CC) -O3 luatest.c -I../lua/5.1.5/include -L../luajit/src -lluajit-5.1 -o build/test

run: konpacto
	cd build && main.exe

.PHONY: all run konpacto luatest lua

