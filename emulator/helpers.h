#ifndef HELPERS_H
#define HELPERS_H

// Helpers for use by the emulation

#ifdef GRID_EMULATION

#include <SDL.h>
#include <cstdint>

// Register event pump once via set_pump(), then call delay(ms).
namespace GRID
{
    using PumpFn = void (*)();    // void pump(); should call SDL_PollEvent loop and handle quit/LED-mode toggle
    using PresentFn = void (*)(); // optional: void present(); calls SDL_RenderPresent if desired

    inline PumpFn &pump_ref()
    {
        static PumpFn pump = nullptr;
        return pump;
    }
    inline PresentFn &present_ref()
    {
        static PresentFn present = nullptr;
        return present;
    }

    inline void set_pump(PumpFn pump) { pump_ref() = pump; }
    inline void set_present(PresentFn pres) { present_ref() = pres; }

    // Core helper
    inline void cooperative_delay(uint32_t ms,
                                  uint32_t min_present_interval_ms = 16)
    {
        const uint64_t freq = SDL_GetPerformanceFrequency();
        const uint64_t start = SDL_GetPerformanceCounter();
        uint64_t last_present = start;

        while (true)
        {
            if (auto pump = pump_ref())
                pump(); // process SDL events: quit, LED-mode toggle, etc.

            const uint64_t now = SDL_GetPerformanceCounter();
            const double elapsed_ms = 1000.0 * (now - start) / freq;
            if (elapsed_ms >= ms)
                break;

            // Keep UI responsive by presenting periodically if a hook is set
            if (auto present = present_ref())
            {
                const double since_present_ms = 1000.0 * (now - last_present) / freq;
                if (since_present_ms >= static_cast<double>(min_present_interval_ms))
                {
                    present();
                    last_present = SDL_GetPerformanceCounter();
                }
            }

            const double remaining_ms = ms - elapsed_ms;
            const uint32_t slice = static_cast<uint32_t>(remaining_ms < 4.0 ? remaining_ms : 4.0);
            SDL_Delay(slice); // cooperative sleep
        }
    }

    // Simple blocking delay without event pumping
    inline void blocking_delay(uint32_t ms)
    {
        SDL_Delay(ms);
    }
} // namespace GRID

    // Use cooperative delay by default in the emulator
    inline void delay(uint32_t ms) { GRID::blocking_delay(ms); }

#endif // GRID_EMULATION
#endif // HELPERS_H
