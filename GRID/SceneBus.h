#ifndef SCENE_BUS_H
#define SCENE_BUS_H
#include <functional>

struct SceneBus
{
    std::function<void()> toMenu;
    std::function<void()> toExample;
    std::function<void()> toBoids;
    std::function<void()> toCalibration;
};

#endif
