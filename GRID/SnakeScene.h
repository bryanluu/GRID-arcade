#ifndef SNAKE_SCENE_H
#define SNAKE_SCENE_H

#include "Scene.h"
#include <bitset>


class SnakeScene : public Scene
{
public:
    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Snake; }
    const char *label() const override { return "Snake"; }
};

#endif // SNAKE_SCENE_H
