#ifndef HELPERS_H
#define HELPERS_H

// Helpers for use by the emulation

#ifdef GRID_EMULATION

#include <SDL.h>
#include <cstdint>

using millis_t = uint32_t;

// Mimic the Arduino's delay function
inline void delay(millis_t ms) { SDL_Delay(ms); }

#else

#include <Arduino.h>

#endif // GRID_EMULATION
#endif // HELPERS_H
