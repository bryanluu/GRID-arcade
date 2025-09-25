#include "Scene.h"

Scene::Scene() : m_name { "_" }
{
}

void Scene::start()
{
    if (Scene::s_currentScene != this)
        Scene::s_currentScene = this;
}

void Scene::run()
{
    // do nothing
}