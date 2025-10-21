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
#include "StartScene.h"

// App manages the current scene; only holds Matrix32&
class App
{
    AppContext ctx;
    std::unique_ptr<Scene> current;
    SceneBus bus{};

    // --- Pause Menu state ---
    millis_t pauseHoldStartMs_ = 0;
    bool pausePrevPressed_ = false;
    bool pauseArmed_ = false;
    bool paused_;
    bool selectQuit_ = false;
    static constexpr millis_t PAUSE_TRIGGER_MS = 5000;
    // Basic nav: X left/right to change selection, button to activate
    // Simple hysteresis with thresholds
    static constexpr float HYSTERESIS_THRESHOLD = 0.45f;
    static constexpr millis_t SELECT_WAIT = 500; // wait after select for drama
    static constexpr Color333 IDLE_COLOR = Colors::Muted::White;
    static constexpr Color333 SELECT_COLOR = Colors::Bright::White;

    bool checkCurrentSceneCanPause() const
    {
        bool isStart = (current && current->kind() == Scene::SceneKind::Start);
        bool isMenu = (current && current->kind() == Scene::SceneKind::Menu);
        return !(isStart || isMenu);
    }

    // returns true if pause is triggered
    bool checkPauseTrigger()
    {
        const InputState s = ctx.input.state();
        const millis_t now = ctx.time.nowMs();

        if (s.pressed)
        {
            // Rising edge: start timing
            if (!pausePrevPressed_)
            {
                pauseHoldStartMs_ = now;
            }
            // Arm once threshold reached (do not switch yet)
            if (!pauseArmed_ && pauseHoldStartMs_ && (now - pauseHoldStartMs_) >= PAUSE_TRIGGER_MS)
            {
                pauseArmed_ = true;
            }
        }
        else
        {
            // Button released: fire if armed
            if (pauseArmed_)
            {
                pauseArmed_ = false;
                pauseHoldStartMs_ = 0;
                pausePrevPressed_ = s.pressed;

                return true;
            }
            // Not armed -> just cancel timer
            pauseHoldStartMs_ = 0;
        }
        pausePrevPressed_ = s.pressed;
        return false;
    }

    void drawPauseMenu(bool selectQuit, bool left, bool right, bool press)
    {
        ctx.gfx.clear();
        ctx.gfx.setTextSize(1);

        ctx.gfx.setCursor(1, 1);
        if (!selectQuit)
            ctx.gfx.setTextColor((press ? Colors::Bright::Green : SELECT_COLOR));
        else
            ctx.gfx.setTextColor(IDLE_COLOR);
        ctx.gfx.print("Cont.");

        ctx.gfx.setCursor(1, 12);
        if (selectQuit)
            ctx.gfx.setTextColor((press ? Colors::Bright::Red : SELECT_COLOR));
        else
            ctx.gfx.setTextColor(IDLE_COLOR);
        ctx.gfx.print("Quit");

        // arrow hint
        ctx.gfx.setCursor(1, MATRIX_HEIGHT - 8);
        if (left)
            ctx.gfx.setTextColor(Colors::Bright::White);
        else
            ctx.gfx.setTextColor(Colors::Muted::White);
        ctx.gfx.print("<");

        ctx.gfx.setCursor(10, MATRIX_HEIGHT - 8);
        if (press)
            ctx.gfx.setTextColor(Colors::Bright::White);
        else
            ctx.gfx.setTextColor(Colors::Muted::White);
        ctx.gfx.print("OK");

        if (right)
            ctx.gfx.setTextColor(Colors::Bright::White);
        else
            ctx.gfx.setTextColor(Colors::Muted::White);
        ctx.gfx.setCursor(MATRIX_WIDTH - 6, MATRIX_HEIGHT - 8);
        ctx.gfx.print(">");
    }

    void handlePause()
    {
        const InputState s = ctx.input.state();
        // Basic nav: X left/right to change selection, button to activate
        static bool prevLeft = false, prevRight = false, prevPress = false;

        bool left = (s.x < -HYSTERESIS_THRESHOLD);
        bool right = (s.x > HYSTERESIS_THRESHOLD);
        bool press = s.pressed;

        if (left && !prevLeft)
            selectQuit_ = false;
        if (right && !prevRight)
            selectQuit_ = true;

        drawPauseMenu(selectQuit_, left, right, press);

        if (press && !prevPress && ctx.bus)
        {
            // shows center press with a small pause
            ctx.gfx.show();
            ctx.time.sleep(SELECT_WAIT);
            if (selectQuit_)
            {
                // Switch scenes here. If you have a MenuScene route bound on a bus:
                if (ctx.bus && ctx.bus->toMenu)
                    ctx.bus->toMenu();
                else
                    this->setScene<MenuScene>();
            }
            paused_ = false;
        }
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
        ctx.logger.logf(LogLevel::Debug, "Started %s Scene.", current->label());
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
        if (paused_)
            handlePause(); // handles global pause
        else
        {
            if (checkCurrentSceneCanPause() && checkPauseTrigger())
                paused_ = true;
            else
                current->loop(ctx);
        }
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
