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

void pollEvents(bool &running, SDLMatrix32 &gfx)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            running = false; // Q triggers quit
        if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)
                running = false; // ESC also triggers quit
            else if (e.key.keysym.sym == SDLK_l)
                gfx.setLEDMode(!gfx.ledMode()); // toggle LED mode when L is pressed
        }
    }
}

// Main emulation loop
void run_emulation()
{
    SDLMatrix32 gfx{};
    gfx.begin();
    FixedStepTiming time{TICK_HZ};
    App app{gfx, time};

    app.setScene<ExampleScene>();

    bool running = true;
    while (running)
    {
        // 1) Events
        pollEvents(running, gfx);
        // 2) Timing
        int steps = time.pump();
        for (int i = 0; i < steps; ++i)
        {
            app.loopOnce(); // Scene consumes ctx.time
        }
        time.sleep_to_cadence();
    }
}

int main()
{
    run_emulation();
    return 0;
}
