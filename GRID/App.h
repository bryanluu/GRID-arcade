#ifndef APP_H
#define APP_H

#include "AppContext.h"
#include "Helpers.h"
#include "Scene.h"
#include <memory>
#include <utility>

#ifdef GRID_EMULATION
#include "SDLMatrix32.h"
#else
#include "RGBMatrix32.h"
#endif

#include "SceneBus.h"
#include "BoidsScene.h"
#include "CalibrationScene.h"
#include "ExampleScene.h"
#include "MenuScene.h"

// App manages the current scene; only holds Matrix32&
class App
{
    AppContext ctx;
    std::unique_ptr<Scene> current;
    SceneBus bus{};

    // --- Quit-to-menu state ---
    millis_t quitHoldStartMs_ = 0;
    bool quitPrevPressed_ = false;
    bool quitArmed_ = false;
    static constexpr millis_t QUIT_HOLD_MS = 5000;

    bool checkQuitToMenu_()
    {
        const InputState s = ctx.input.state();
        const millis_t now = ctx.time.nowMs();

        if (s.pressed)
        {
            // Rising edge: start timing
            if (!quitPrevPressed_)
            {
                quitHoldStartMs_ = now;
            }
            // Arm once threshold reached (do not switch yet)
            if (!quitArmed_ && quitHoldStartMs_ && (now - quitHoldStartMs_) >= QUIT_HOLD_MS)
            {
                quitArmed_ = true;
            }
        }
        else
        {
            // Button released: fire if armed
            if (quitArmed_)
            {
                quitArmed_ = false;
                quitHoldStartMs_ = 0;
                quitPrevPressed_ = s.pressed;

                // Switch scenes here. If you have a MenuScene route bound on a bus:
                if (ctx.bus && ctx.bus->toMenu)
                    ctx.bus->toMenu();
                else
                    this->setScene<MenuScene>();
                return true;
            }
            // Not armed -> just cancel timer
            quitHoldStartMs_ = 0;
        }
        quitPrevPressed_ = s.pressed;
        return false;
    }

public:
    explicit App(Matrix32 &gfx, Timing &time, Input &input, ILogger &logger, IStorage &storage) : ctx{gfx, time, input, logger, storage}
    {
        ctx.bus = &bus;
        // Bind routes. Lambdas capture this App and call setScene.
        bus.toMenu = [this]
        { this->setScene<MenuScene>(); };
        bus.toExample = [this]
        { this->setScene<ExampleScene>(); };
        bus.toBoids = [this]
        { this->setScene<BoidsScene>(); };
        bus.toCalibration = [this]
        { this->setScene<CalibrationScene>(); };
    }

    // Replace the current scene with a newly constructed SceneT.
    // - Destroys the old scene, creates SceneT(args...), then calls setup(gfx).
    // - Perfect-forwards args to SceneT's ctor (no unnecessary copies).
    // - Fails to compile if SceneT is not compatible with Scene or ctor args.
    template <typename SceneT, typename... Args>
    void setScene(Args &&...args)
    {
        static_assert(std::is_base_of<Scene, SceneT>::value, "SceneT must derive from Scene");
        current.reset(new SceneT(std::forward<Args>(args)...));
        // apply preferred timing
        auto prefs = current->timingPrefs();
        ctx.time.applyPreference(prefs);
        ctx.time.resetSceneClock();
        // immediate only during setup (works on SDLMatrix32, no-op on Arduino)
        ctx.gfx.setImmediate(true);
        ctx.gfx.clear();
        current->setup(ctx);
        ctx.gfx.setImmediate(false);
    }

    // Call the current scene's loop(ctx).
    // dt is in milliseconds.
    void loopOnce()
    {
        ctx.input.sample();
        if (checkQuitToMenu_())
            return; // handle global quit before scene logic
        current->loop(ctx);
        ctx.gfx.show();
    }

    // Logs helpful diagnostics: FPS, plus calibration values
    void logDiagnostics()
    {
        float fps = ctx.time.fps();
        InputState input = ctx.input.state();
        // Log FPS
        ctx.logger.logf(LogLevel::Debug, "FPS: %5.2f", fps);
        ctx.logger.flush();
        // Log inputs
        ctx.logger.logf(LogLevel::Debug, "Raw X: %d Y: %d, Norm X: %5.3f Y: %5.3f, Pressed: %d",
                        input.x_adc, input.y_adc,
                        input.x, input.y,
                        input.pressed ? 1 : 0);
    }
};

#endif
