#include "Scene.h"

// Define statics
Scene* Scene::currentScene = nullptr;
App*   Scene::app = nullptr;

Scene::Scene() : name{"_"} {}

void Scene::start()
{
    if (Scene::currentScene != this)
        Scene::currentScene = this;
}

void Scene::run()
{
    // do nothing
}
