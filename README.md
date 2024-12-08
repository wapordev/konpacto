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