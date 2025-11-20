#ifndef LIFE_SCENE_H
#define LIFE_SCENE_H

#include "Scene.h"
#include <bitset>

class LifeScene final : public Scene
{
    std::bitset<MATRIX_WIDTH * MATRIX_HEIGHT> cells;

public:
    SceneKind kind() const override { return SceneKind::Life; }
    const char *label() const override { return "Life"; }
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // LIFE_SCENE_H
