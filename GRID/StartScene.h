#ifndef GRID_STARTSCENE_H
#define GRID_STARTSCENE_H

#include "Scene.h"
#include "ScrollTextHelper.h"

class StartScene final : public Scene
{
public:
    StartScene() = default;

    SceneTimingPrefs timingPrefs() const override
    {
        // Default 60Hz is fine; return NaN to keep global default
        return SceneTimingPrefs(std::numeric_limits<double>::quiet_NaN());
    }

    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;

private:
    ScrollText banner_;
};

#endif // GRID_STARTSCENE_H
