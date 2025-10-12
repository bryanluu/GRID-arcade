#ifndef CALIBRATION_SCENE_H
#define CALIBRATION_SCENE_H

#include "Scene.h"
#include "Colors.h"

/**
 * @brief A simple scene to help calibrate the joystick
 */
class CalibrationScene final : public Scene
{
    static const int HOLD_TO_START_MS = 2'000; // ms
    static const int CIRCLE_CENTER = 15;
    static const int CIRCLE_RADIUS = 15;
    static constexpr Color333 CIRCLE_COLOR = GRAY;
    static constexpr Color333 PRESSED_COLOR = Color333{0, 1, 0};
    static constexpr Color333 IDLE_COLOR = Color333{1, 0, 0};
    static constexpr Color333 PRESSED_CURSOR_COLOR = GREEN;
    static constexpr Color333 IDLE_CURSOR_COLOR = RED;

    millis_t lastPressTime;

public:
    CalibrationScene() : lastPressTime{UINT32_MAX} {}
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // CALIBRATION_SCENE_H
