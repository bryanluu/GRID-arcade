# GRID-Arcade

This is the **GRID** (**G**aming **R**etro **I**nteractive **D**evice), my fun hobby project to create a custom arcade box.

## Components
The display is a [32x32 RGB LED matrix](https://www.adafruit.com/product/1484) connected with an [Adafruit Metro M0 (SAMD21)](https://www.adafruit.com/product/3505) equipped with a [Adafruit RGB Matrix Shield](https://www.adafruit.com/product/2601). The unit is controlled by a PS2 Joystick hooked up to the Metro.

Everything is housed in a custom unit built in collaboration with my friend Herman Tabunganua.

## Software
The GRID arcade can also be run as a desktop emulation. The software for each target is identical, in the `App` class.

The emulation uses a simple Makefile with debug-friendly targets.

### Prereqs
- SDL2 development headers and libs
  - Linux: `sudo apt install libsdl2-dev` (or use `sdl2-config` / `pkg-config`)
- g++ with C++17

### Targets

- `make`  
  Release-ish build (O2).

- `make DEBUG=1` or `make debug`  
  Debug build with symbols, AddressSanitizer, and `-DDEBUG`.  
  Flags include: `-g -O0 -fno-omit-frame-pointer -fsanitize=address`.

- `make run`  
  Build then run `./build/grid`.

- `make run-debug`  
  Build with `DEBUG=1` then run.

- `make clean`  
  Remove the `build/` folder.

### Notes
- SDL flags are discovered via `pkg-config sdl2` or fall back to `sdl2-config`.
- On debug builds, ASan is enabled for both compile and link. If you need to disable leak reports temporarily:
```shell
ASAN_OPTIONS=detect_leaks=0 ./build/grid
```
