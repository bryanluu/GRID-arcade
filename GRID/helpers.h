#ifndef HELPERS_H
#define HELPERS_H

// Math
#define BOUND(l, x, h) ((x) > (h) ? (h) : ((x) < (l) ? (l) : (x))) // return x bounded between l and h

// Helpers for use by the emulation
#ifdef GRID_EMULATION

#include <SDL.h>
#include <cstdint>


using byte = uint8_t; // Mimic the byte alias in Arduino-land
using millis_t = uint32_t; // convenience alias for time in milliseconds

// Provide a random int between 0..range
inline int random(int range) { return static_cast<float>(rand()) * range / RAND_MAX; }

// Mimic the Arduino's delay function
inline void delay(millis_t ms) { SDL_Delay(ms); }

#else

#include <Arduino.h>

#endif // GRID_EMULATION
#endif // HELPERS_H
