# konpacto
 Portable FM Macro tracker

---

as of 8 Dec 2024, compiled with https://github.com/XK9274/sdl2_miyoo/tree/onion

```
git clone https://github.com/XK9274/sdl2_miyoo/tree/master
cd sdl2_miyoo
rm -rf sdl2
git clone -b vanilla https://github.com/XK9274/sdl2_miyoo/tree/master
mv sdl2_miyoo sdl2
```

small steps were omitted from this fork, like `cd nds_miyoo` after creating the container, and `apt-get install autoconf` as well as `cd swiftshader && mkdir build`.

It also seems to be missing libjson-c library, im not smart enough to know where to find it so i took it from the pico-8 wrapper app.

Ive cloned this repo into sdl2_miyoo, and added this to the Makefile
```
.PHONY: konpacto
konpacto:
	mkdir konpacto/build
	mkdir konpacto/build/libs
	cp swiftshader/build/*.so konpacto/build/libs/
	cp sdl2/build/.libs/libSDL2-2.0.so.0 konpacto/build/libs/
	$(CC) konpacto/src/main.c -I/opt/mmiyoo/arm-buildroot-linux-gnueabihf/sysroot/usr/include/SDL2 -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_gfx -lSDL2_ttf -o konpacto/build/main
```