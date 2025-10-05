#ifndef EXAMPLE_SCENE_H
#define EXAMPLE_SCENE_H

#include "AppContext.h"
#include "Scene.h"
#include "helpers.h"

class ExampleScene final : public Scene
{
public:
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif
