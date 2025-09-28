#include "Scene.h"

Scene *Scene::currentScene = nullptr;

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
