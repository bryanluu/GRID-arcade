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

// App manages the current scene; only holds Matrix32&
class App
{
    AppContext ctx;
    std::unique_ptr<Scene> current;

public:
    explicit App(Matrix32 &gfx, Timing &time, Input &input, ILogger &logger) : ctx{gfx, time, input, logger} {}

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
        current->setup(ctx);
        ctx.gfx.setImmediate(false);
    }

    // Call the current scene's loop(ctx).
    // dt is in milliseconds.
    void loopOnce()
    {
        ctx.input.sample();
        current->loop(ctx);
        ctx.gfx.show();
    }
};

#endif
