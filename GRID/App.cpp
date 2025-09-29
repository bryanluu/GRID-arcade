#include "App.h"
#include "ExampleScene.h"
#include "BoidsScene.h"

App* App::instance = nullptr;  // define the static
ExampleScene exampleScene = ExampleScene(); // this needs to be a global
BoidsScene boidsScene = BoidsScene();

App::App(Matrix32& matrix) : matrix(matrix) 
{
    if (App::instance) return;

    App::instance = this;
    Scene::app = this;
}

void App::setup()
{
    Scene::currentScene = &boidsScene;
    Scene::currentScene->start();
}

void App::loop(uint32_t now)
{
    Scene::currentScene->run();
}
