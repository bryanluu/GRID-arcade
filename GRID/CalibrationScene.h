#ifndef CALIBRATION_SCENE_H
#define CALIBRATION_SCENE_H

#include "Scene.h"
#include "Colors.h"

/**
 * CalibrationScene: 5-stage timed joystick calibration driven by AppContext timing.
 * Hold button ≥2s to start, progress through stages; button cancels.
 * No persistence here — caller reads finished() + getResult() and saves externally.
 */
class CalibrationScene final : public Scene
{
    static const int HOLD_TO_START_MS = 2000;
    static const int STAGE_MS = 3000;
    static const millis_t TRANSITION_BUFFER = 1000; // buffer between state transitions
    static const int CIRCLE_CENTER = 15;
    static const int CIRCLE_RADIUS = 15;
    static constexpr Color333 CIRCLE_COLOR = GRAY;
    static constexpr Color333 PRESSED_COLOR = Color333{0, 1, 0};
    static constexpr Color333 IDLE_COLOR = Color333{1, 0, 0};
    static constexpr Color333 PRESSED_CURSOR_COLOR = GREEN;
    static constexpr Color333 IDLE_CURSOR_COLOR = RED;

    // State machine: Idle → Left → Right → Up → Down → Center → Done or Canceled.
    // Pressing the button during any stage aborts with no changes (safety).
    enum State
    {
        Idle,
        StageLeft,
        StageRight,
        StageUp,
        StageDown,
        StageCenter,
        Done,
        Canceled
    } state_ = Idle;

    InputCalibration staged_calib;
    millis_t hold_start_{0};
    millis_t stage_start_{0};

    // extremum trackers and center accumulators
    int x_min_ = InputCalibration::ADC_MAX, x_max_ = InputCalibration::ADC_MIN;
    int y_min_ = InputCalibration::ADC_MAX, y_max_ = InputCalibration::ADC_MIN;
    int x_acc_ = 0, y_acc_ = 0, count_ = 0;

    void drawCalibrationCross(AppContext &ctx);
    const char *stageLabel(State s) const;
    void handleIdle(AppContext &ctx);
    void beginStage(AppContext &ctx, State s);
    void drawStage(AppContext &ctx);
    void handleStage(AppContext &ctx);
    void handleDone(AppContext &ctx);
    void handleCanceled(AppContext &ctx);

public:
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
    bool finished() const
    {
        return state_ == Done;
    }
    const InputCalibration &getResult() const
    {
        return staged_calib;
    }
};

#endif // CALIBRATION_SCENE_H
