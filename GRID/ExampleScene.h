#ifndef EXAMPLE_SCENE_H
#define EXAMPLE_SCENE_H

#include "Scene.h"
#include "helpers.h"

class ExampleScene final : public Scene
{
public:
    void setup(Matrix32 &gfx) override;
    void loop(Matrix32 &gfx, millis_t dt) override;
};

#endif
