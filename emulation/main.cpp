#include "App.h"
#include "BoidsScene.h"
#include "ExampleScene.h"
#include "FixedStepTiming.h"
#include "SDLInputProvider.h"
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
    SDLInputProvider inputProvider{};
    SDL_Window *win = gfx.window();
    bool running = true; // main loop flag

    // TODO handle init failure
    if (!inputProvider.init(win))
        return; // failed to init input provider

    // Wire quit (Q/Esc) and LED toggle (L)
    inputProvider.onQuit([&]
                         { running = false; });
    // toggle LED mode when L is pressed
    inputProvider.onToggleLED([&]
                              { gfx.toggleLEDMode(); });

    Input input{};
    input.init(&inputProvider);
    App app{gfx, time, input};

    app.setScene<BoidsScene>();

    while (running)
    {
        // 1) Events
        // pollEvents(running, gfx);
        // 2) Timing
        int steps = time.pump();
        for (int i = 0; i < steps; ++i)
        {
            inputProvider.pumpEvents(); // handle input events
            app.loopOnce();             // Scene consumes ctx.time
        }
        // Log inputs
        printf("X: %5.3f Y: %5.3f Pressed: %d\n",
               input.state().x, input.state().y, input.state().pressed ? 1 : 0);
        // Log FPS
        printf("FPS: %5.2f\n", time.fps());
        fflush(stdout);
        // 3) Sleep to cadence
        time.sleep_to_cadence();
    }
}

int main()
{
    run_emulation();
    return 0;
}
