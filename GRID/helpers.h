#ifndef HELPERS_H
#define HELPERS_H

// Helpers for use by the emulation

#ifdef GRID_EMULATION

#include <SDL.h>
#include <cstdint>

// Mimic the Arduino's delay function
inline void delay(uint32_t ms) { SDL_Delay(ms); }

#else

#include <Arduino.h>

#endif // GRID_EMULATION
#endif // HELPERS_H
