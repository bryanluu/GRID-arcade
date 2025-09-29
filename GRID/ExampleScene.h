#ifndef EXAMPLE_SCENE_H
#define EXAMPLE_SCENE_H

#include "Scene.h"

class ExampleScene : public Scene
{
public:
    ExampleScene() : Scene()
    {
        name = "Example";
    }

    void start();
    void run();
};

#endif
