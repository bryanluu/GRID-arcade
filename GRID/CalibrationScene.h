#ifndef CALIBRATION_SCENE_H
#define CALIBRATION_SCENE_H

#include "Scene.h"
#include "Colors.h"

/**
 * @brief A simple scene to help calibrate the joystick
 */
class CalibrationScene final : public Scene
{
    static const int circleCenter = 15;
    static const int circleRadius = 15;
    static constexpr Color333 outlineColor = GRAY;
    static constexpr Color333 cursorColor = BLUE;

public:
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // CALIBRATION_SCENE_H
