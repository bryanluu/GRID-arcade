#ifndef BONNEH_SCENE_H
#define BONNEH_SCENE_H

#include "Scene.h"

class BonnehScene : public Scene
{
public:
    BonnehScene() = default;
    ~BonnehScene() override = default;
    SceneKind kind() const override { return SceneKind::Bonneh; }
    const char *label() const override { return "Bonneh"; }
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // BONNEH_SCENE_H
