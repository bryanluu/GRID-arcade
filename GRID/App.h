#ifndef APP_H
#define APP_H

#include "helpers.h"
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
    Matrix32 &gfx;
    std::unique_ptr<Scene> current;

public:
    explicit App(Matrix32 &g) : gfx(g) {}

    // Replace the current scene with a newly constructed SceneT.
    // - Destroys the old scene, creates SceneT(args...), then calls setup(gfx).
    // - Perfect-forwards args to SceneT's ctor (no unnecessary copies).
    // - Fails to compile if SceneT is not compatible with Scene or ctor args.
    template <typename SceneT, typename... Args>
    void setScene(Args &&...args)
    {
        static_assert(std::is_base_of<Scene, SceneT>::value, "SceneT must derive from Scene");
        current.reset(new SceneT(std::forward<Args>(args)...));
        
        // Immediate only during setup (works on SDLMatrix32, no-op on Arduino)
        gfx.setImmediate(true);
        current->setup(gfx);
        gfx.setImmediate(false);
    }

    // Call the current scene's loop(gfx, dt).
    // dt is in milliseconds.
    void loopOnce(millis_t dt)
    {
        current->loop(gfx, dt);
    }
};

#endif
