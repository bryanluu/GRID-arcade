#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>
#include <cmath>

// Type 
using millis_t = uint32_t; // convenience alias for time in milliseconds

// These helpers are needed because the defs are missing for either platform
namespace Helpers
{
    // Clamp value v to the range [lo..hi]
    template <typename T>
    inline T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); } // Arduino lacks std::clamp
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
