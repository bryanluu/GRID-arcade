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
