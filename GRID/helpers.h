#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>
#include <cmath>

// Math
#define BOUND(l, x, h) ((x) > (h) ? (h) : ((x) < (l) ? (l) : (x))) // return x bounded between l and h


// Type 
using millis_t = uint32_t; // convenience alias for time in milliseconds

namespace Helpers
{
    // Provide a random int between 0..range
    inline int random(int range) { return static_cast<float>(rand()) * range / RAND_MAX; }
    inline int random() { return rand();}
}

// Helpers for use by the emulation
#ifdef GRID_EMULATION

#include <SDL.h>

using byte = uint8_t; // Mimic the byte alias in Arduino-land

#else

#include <Arduino.h>

#endif // GRID_EMULATION
#endif // HELPERS_H
