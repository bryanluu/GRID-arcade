#include "App.h"
#include "EmulationLogger.h"
#include "FileStorage.h"
#include "FixedStepTiming.h"
#include "IStorage.h"
#include "SDLInputProvider.h"
#include "SDLMatrix32.h"
#include "StartScene.h"
#include <SDL.h>
#include <algorithm>
#include <cstdint>
#include <chrono>

#include "SnakeScene.h"

// match GRID hardware
static constexpr double TICK_HZ = 60.0;

// Main emulation loop
void run_emulation()
{
    // unsigned long seed = static_cast<unsigned long>(
    //     std::chrono::high_resolution_clock::now().time_since_epoch().count());
    unsigned long seed = 1l; // use consistent seed for emulation testing
    Helpers::randomSeed(seed);

    FileStorage storage;
    SDLMatrix32 gfx{};
    gfx.begin();
    StdoutSink sink;
    FixedStepTiming timing{TICK_HZ};
    EmulationLogger logger(timing, sink);

    InputCalibration calib = SDLInputProvider::defaultCalib;
    const char *baseDir = "save";
    storage.init(baseDir, &logger);
    calib.load(storage, logger); // load any saved calib
    SDLInputProvider inputProvider{calib};

    SDL_Window *win = gfx.window();
    bool running = true; // main loop flag
    millis_t log_last_ms{};
    millis_t now_ms{};

    if (!inputProvider.init(win))
    {
        logger.logf(LogLevel::Warning, "Failed to initialize emulator input");
        return; // failed to init input provider
    }

    // Wire quit (Q/Esc) and LED toggle (L)
    inputProvider.onQuit([&]
                         { running = false; });
    // toggle LED mode when L is pressed
    inputProvider.onToggleLED([&]
                              { gfx.toggleLEDMode(); });
    // resize window updates LED size
    inputProvider.onResize([&]
                           { gfx.recomputeScale(); });

    Input input{};
    input.init(&inputProvider);
    App app{gfx, timing, input, logger, storage};

    app.setScene<SnakeScene>();

    while (running)
    {
        int steps = timing.pump();
        for (int i = 0; i < steps; ++i)
        {
            inputProvider.pumpEvents(); // handle input events
            app.loopOnce();             // Scene consumes ctx.timing
        }

        now_ms = timing.nowMs();
        if (now_ms - log_last_ms >= timing.MILLIS_PER_SEC)
        {
            app.logDiagnostics();
            log_last_ms = now_ms;
        }
        timing.sleep_to_cadence();
    }
}

int main()
{
    run_emulation();
    return 0;
}
