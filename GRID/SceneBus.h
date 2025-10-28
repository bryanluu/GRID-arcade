#ifndef SCENE_BUS_H
#define SCENE_BUS_H
#include "Scene.h"
#include <functional>

struct SceneBus
{
    std::function<void()> toMenu;
    std::function<void()> toMaze;
    std::function<void()> toBoids;
    std::function<void()> toCalibration;
    std::function<void(Scene::SceneKind, const char *, int)> toSaveScore;
};

#endif
