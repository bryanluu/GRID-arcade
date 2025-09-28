#include "App.h"
#include "ExampleScene.h"

App::App(Matrix32& matrix) : matrix(matrix) 
{
    if (App::instance) return;

    App::instance = this;
    Scene::app = this;
}

void App::setup()
{
    ExampleScene exampleScene = ExampleScene();
    Scene::currentScene = &exampleScene;
    Scene::currentScene->start();
}

void App::loop(uint32_t now)
{
    Scene::currentScene->run();
}
