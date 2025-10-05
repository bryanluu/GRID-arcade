#include "App.h"
#include "BoidsScene.h"
#include "ExampleScene.h"
#include "FixedStepTiming.h"
#include "SDLMatrix32.h"
#include <SDL.h>
#include <algorithm>
#include <cstdint>

// match GRID hardware
static constexpr double TICK_HZ = 60.0;

void run_emulator()
{
    using u64 = unsigned long long;
    SDLMatrix32 gfx{};
    FixedStepTiming time{TICK_HZ};
    App app{gfx, time};
    gfx.begin();

    app.setScene<BoidsScene>();

    // We pace manually; disable vsync so Present doesn't block unpredictably
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");

    bool running = true;
    const u64 freq = SDL_GetPerformanceFrequency();
    u64 now = SDL_GetPerformanceCounter();
    double accumulator = 0.0;
    while (running)
    {
        // 1) Events
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)
                    running = false;
                else if (e.key.keysym.sym == SDLK_l)
                    gfx.setLEDMode(!gfx.ledMode());
            }
        }

        // 2) Time step accumulation
        u64 newNow = SDL_GetPerformanceCounter();
        double frameSec = double(newNow - now) / double(freq);
        now = newNow;

        // Clamp very large pauses (drag window, breakpoint) to avoid spirals
        frameSec = std::min(frameSec, 0.25);
        accumulator += frameSec;

        int steps = time.pump();
		for (int i = 0; i < steps; ++i) {
			app.loopOnce(); // Scene consumes ctx.time
		}
        gfx.show();
        time.sleep_to_cadence();
    }
}

int main()
{
    run_emulator();
    return 0;
}
