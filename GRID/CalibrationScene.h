#ifndef CALIBRATION_SCENE_H
#define CALIBRATION_SCENE_H

#include "Scene.h"
#include "Colors.h"

/**
 * @brief A simple scene to help calibrate the joystick
 */
class CalibrationScene final : public Scene
{
    static const int triggerDuration = 2000; // ms
    static const int circleCenter = 15;
    static const int circleRadius = 15;

    millis_t lastPressTime;

public:
    CalibrationScene() : lastPressTime{UINT32_MAX} {}
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // CALIBRATION_SCENE_H
