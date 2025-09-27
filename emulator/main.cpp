#include "App.h"
#include "SDLMatrix32.h"
#include <SDL.h>
#include <cstdint>

// match GRID hardware
static constexpr int    TICK_HZ = 60;
static constexpr double DT_SEC  = 1.0 / TICK_HZ;

static uint32_t millis_now(uint32_t start) { return SDL_GetTicks() - start; }

void runEmulator(App& app) {
    using u64 = unsigned long long;

    // We pace manually; disable vsync so Present doesn't block unpredictably
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");

    bool running = true;
    const u64 freq = SDL_GetPerformanceFrequency();
    u64 now = SDL_GetPerformanceCounter();
    double accumulator = 0.0;

    app.setup(); // Arduino-style setup once
    SDLMatrix32& matrix = dynamic_cast<SDLMatrix32&>(app.m);

    while (running) {
        // 1) Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)
                    running = false;
                else if (e.key.keysym.sym == SDLK_l) 
                    matrix.setLEDMode(!matrix.ledMode());
            }
        }

        // 2) Time step accumulation
        u64 newNow = SDL_GetPerformanceCounter();
        double frameSec = double(newNow - now) / double(freq);
        now = newNow;

        // Clamp very large pauses (drag window, breakpoint) to avoid spirals
        frameSec = std::min(frameSec, 0.25);
        accumulator += frameSec;

        // 3) Fixed updates at TICK_HZ
        // Optional: cap catch-up iterations to keep UI responsive
        int maxSteps = 5;
        while (accumulator >= DT_SEC && maxSteps-- > 0) {
            uint32_t millis_now = (uint32_t)(newNow * 1000.0 / double(freq));
            app.loop(millis_now);
            accumulator -= DT_SEC;
        }

        // 4) Render
        matrix.show();

        // 5) Sleep a bit to target cadence (tiny margin to avoid oversleep)
        double frameLeft = DT_SEC - accumulator;
        if (frameLeft > 0) {
            double sleepSec = std::max(0.0, frameLeft - 0.001);
            SDL_Delay((Uint32)(sleepSec * 1000.0));
        }
    }
}

int main()
{
    SDLMatrix32 m;   // your concrete Matrix32 implementation
    App app{m};
    runEmulator(app);
    return 0;
}
