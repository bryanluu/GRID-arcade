#include "App.h"
#include "BoidsScene.h"
#include "CalibrationScene.h"
#include "EmulationLogger.h"
#include "ExampleScene.h"
#include "FileStorage.h"
#include "FixedStepTiming.h"
#include "IStorage.h"
#include "SDLInputProvider.h"
#include "SDLMatrix32.h"
#include <SDL.h>
#include <algorithm>
#include <cstdint>

// match GRID hardware
static constexpr double TICK_HZ = 60.0;

// Smoke test using current working directory/save
static void run_filestorage_smoke(FileStorage &storage, ILogger &logger)
{
    const char *baseDir = "save"; // creates ./save next to the emulator
    if (!storage.init(baseDir, &logger))
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] init failed");
        return;
    }

    const char *fname = "emu_test.bin"; // stored under ./save
    const char payload[] = "hello-grid";
    if (!storage.writeAll(fname, payload, sizeof(payload)))
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] writeAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FileStorageTest] write ok");

    char buf[64] = {};
    auto r = storage.readAll(fname, buf, sizeof(buf));
    if (!r)
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] readAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FileStorageTest] read %d bytes, contents='%s'", r.bytes, buf);

    // Clean up
    if (!storage.removeFile(fname))
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] removeFile failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FileStorageTest] removed ok");
}

// Main emulation loop
void run_emulation()
{
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

    app.setScene<CalibrationScene>();

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
