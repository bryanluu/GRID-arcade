#include "App.h"
#include "BoidsScene.h"
#include "CalibrationScene.h"
#include "ExampleScene.h"
#include "FixedStepTiming.h"
#include "Logger.h"
#include "SDLInputProvider.h"
#include "SDLMatrix32.h"
#include <SDL.h>
#include <algorithm>
#include <cstdint>

// match GRID hardware
static constexpr double TICK_HZ = 60.0;

// Main emulation loop
void run_emulation()
{
    SDLMatrix32 gfx{};
    gfx.begin();
    DesktopSink sink;
    FixedStepTiming time{TICK_HZ};
    Logger logger(time, sink);
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

    app.setScene<CalibrationScene>();

    while (running)
    {
        int steps = time.pump();
        for (int i = 0; i < steps; ++i)
        {
            inputProvider.pumpEvents(); // handle input events
            app.loopOnce();             // Scene consumes ctx.time
        }
        // Log inputs
        logger.log(LogLevel::Debug, "Raw X: %d Y: %d, Norm X: %5.3f Y: %5.3f, Pressed: %d",
                   input.state().x_adc, input.state().y_adc,
                   input.state().x, input.state().y,
                   input.state().pressed ? 1 : 0);
        // Log FPS
        logger.log(LogLevel::Debug, "FPS: %5.2f", time.fps());
        logger.flush();
        time.sleep_to_cadence();
    }
}

int main()
{
    run_emulation();
    return 0;
}
