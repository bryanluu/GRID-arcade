#include "CalibrationScene.h"
#include "Colors.h"
#include "Helpers.h"
#include "Input.h"
#include "ScrollTextHelper.h"
#include <algorithm>
#include <cstring>

constexpr Color333 CalibrationScene::CIRCLE_COLOR;
constexpr Color333 CalibrationScene::PRESSED_COLOR;
constexpr Color333 CalibrationScene::IDLE_COLOR;
constexpr Color333 CalibrationScene::PRESSED_CURSOR_COLOR;
constexpr Color333 CalibrationScene::IDLE_CURSOR_COLOR;

void CalibrationScene::setup(AppContext &ctx)
{
    staged_calib = ctx.input.getCalibration();
    ctx.gfx.setImmediate(true);
    int ts = 1;
    ctx.gfx.setTextSize(ts);
    Color333 tc = WHITE;
    ctx.gfx.setTextColor(tc);

    ctx.gfx.setCursor(1, 1);
    ctx.gfx.print("Press");
    ctx.gfx.setCursor(1, 10);
    ctx.gfx.print("2s to");
    ctx.gfx.setCursor(1, 19);
    ctx.gfx.print("Calib");

    ctx.time.sleep(STAGE_MS);
    ctx.gfx.setImmediate(false);
}

void CalibrationScene::drawCalibrationCross(AppContext &ctx)
{
    ctx.gfx.clear();

    // draw a circle for possible joystick output range
    ctx.gfx.drawCircle(CIRCLE_CENTER, CIRCLE_CENTER, CIRCLE_RADIUS, CIRCLE_COLOR);

    // draw the axis cross
    ctx.gfx.drawLine(0, CIRCLE_RADIUS, 2 * CIRCLE_RADIUS, CIRCLE_RADIUS, CIRCLE_COLOR);
    ctx.gfx.drawLine(CIRCLE_RADIUS, 0, CIRCLE_RADIUS, 2 * CIRCLE_RADIUS, CIRCLE_COLOR);

    // draw the cursor
    InputState state = ctx.input.state();
    int cursorX = round(CIRCLE_CENTER + state.x * CIRCLE_RADIUS);
    int cursorY = round(CIRCLE_CENTER + state.y * CIRCLE_RADIUS);
    ctx.gfx.drawLine(CIRCLE_CENTER, CIRCLE_CENTER,
                     cursorX, cursorY,
                     state.pressed ? PRESSED_COLOR : IDLE_COLOR);
    ctx.gfx.drawPixel(cursorX, cursorY, state.pressed ? PRESSED_CURSOR_COLOR : IDLE_CURSOR_COLOR);
}

const char *CalibrationScene::stageLabel(State s) const
{
    switch (s)
    {
    case StageLeft:
        return "Left";
    case StageRight:
        return "Right";
    case StageUp:
        return "Up";
    case StageDown:
        return "Down";
    case StageCenter:
        return "Mid";
    default:
        return "";
    }
}

void CalibrationScene::handleIdle(AppContext &ctx)
{
    drawCalibrationCross(ctx);

    // Begin calibration if button is pressed then let go
    // after holding for more HOLD_TO_START_MS
    if (ctx.input.state().pressed)
    {
        if (hold_start_ == 0)
            hold_start_ = ctx.time.nowMs();
    }
    else
    {
        if (hold_start_ > 0)
        {
            const millis_t held = ctx.time.nowMs() - hold_start_;
            hold_start_ = 0;
            if (held >= HOLD_TO_START_MS)
                beginStage(ctx, StageLeft);
        }
    }
}

// Begin a timed capture window. Reset extremum/average trackers.
// We capture extrema for edges and a mean for center to reduce jitter.
void CalibrationScene::beginStage(AppContext &ctx, State s)
{
    state_ = s;
    stage_start_ = ctx.time.nowMs();
    x_min_ = InputCalibration::ADC_MAX;
    x_max_ = InputCalibration::ADC_MIN;
    y_min_ = InputCalibration::ADC_MAX;
    y_max_ = InputCalibration::ADC_MIN;
    x_acc_ = y_acc_ = 0;
    count_ = 0;
    ctx.gfx.clear();
    ctx.gfx.setCursor(1, 1);

    switch (s)
    {
    case Done:
        ctx.logger.logf(LogLevel::Info, "Calibration complete");
        ctx.gfx.print("Calib");
        ctx.gfx.setCursor(1, 10);
        ctx.gfx.print("Saved");
        break;
    case Canceled:
        ctx.logger.logf(LogLevel::Info, "Calibration canceled");
        ctx.gfx.print("Calib");
        ctx.gfx.setCursor(1, 10);
        ctx.gfx.print("Not");
        ctx.gfx.setCursor(1, 19);
        ctx.gfx.print("Saved");
        break;
    default:
        ctx.logger.logf(LogLevel::Info, "Calibration Stage: %s", stageLabel(state_));
        ctx.gfx.print("Keep");
        ctx.gfx.setCursor(1, 10);
        ctx.gfx.print(stageLabel(state_));
    }
}

void CalibrationScene::drawStage(AppContext &ctx)
{
    const int BAR_START_X = 1;
    const int BAR_START_Y = 29;
    const int BAR_WIDTH = 29;
    const int BAR_HEIGHT = 2;
    constexpr Color333 BAR_COLOR = YELLOW;
    const millis_t elapsed = ctx.time.nowMs() - (stage_start_ + TRANSITION_BUFFER);
    const float progress = float(elapsed) / float(STAGE_MS);
    const int elapsed_width = round(BAR_WIDTH * (1.0f - progress));
    // draw over old bar
    ctx.gfx.fillRect(BAR_START_X, BAR_START_Y, BAR_WIDTH, BAR_HEIGHT, BLACK);
    ctx.gfx.fillRect(BAR_START_X, BAR_START_Y, elapsed_width, BAR_HEIGHT, BAR_COLOR);
}

// Per-tick sample: update min/max and center accumulators.
// Uses O(1) memory — no large buffers or histories.
void CalibrationScene::handleStage(AppContext &ctx)
{
    const millis_t elapsed = ctx.time.nowMs() - stage_start_;
    if (elapsed < TRANSITION_BUFFER)
        return;

    if (ctx.input.state().pressed)
    {
        beginStage(ctx, Canceled);
        return;
    } // cancel on press

    const int ax = ctx.input.state().x_adc;
    const int ay = ctx.input.state().y_adc;
    if (ax < x_min_)
        x_min_ = ax;
    if (ax > x_max_)
        x_max_ = ax;
    if (ay < y_min_)
        y_min_ = ay;
    if (ay > y_max_)
        y_max_ = ay;
    x_acc_ += ax;
    y_acc_ += ay;
    ++count_;

    drawStage(ctx);
    if (elapsed >= TRANSITION_BUFFER + STAGE_MS)
    {
        switch (state_)
        {
        case StageLeft:
            staged_calib.x_adc_low = Helpers::clamp(x_acc_ / count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageRight);
            break;
        case StageRight:
            staged_calib.x_adc_high = Helpers::clamp(x_acc_ / count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageUp);
            break;
        case StageUp:
            staged_calib.y_adc_low = Helpers::clamp(y_acc_ / count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageDown);
            break;
        case StageDown:
            staged_calib.y_adc_high = Helpers::clamp(y_acc_ / count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageCenter);
            break;
        case StageCenter:
            if (count_ > 0)
            {
                staged_calib.x_adc_center = Helpers::clamp(x_acc_ / count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
                staged_calib.y_adc_center = Helpers::clamp(y_acc_ / count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
                // calibrate deadzone
                float x_min_normalized = Input::toNorm(x_min_, staged_calib.x_adc_low, staged_calib.x_adc_center, staged_calib.x_adc_high);
                float x_max_normalized = Input::toNorm(x_max_, staged_calib.x_adc_low, staged_calib.x_adc_center, staged_calib.x_adc_high);
                float x_drift = std::max(fabsf(x_min_normalized), fabsf(x_max_normalized));
                float y_min_normalized = Input::toNorm(y_min_, staged_calib.y_adc_low, staged_calib.y_adc_center, staged_calib.y_adc_high);
                float y_max_normalized = Input::toNorm(y_max_, staged_calib.y_adc_low, staged_calib.y_adc_center, staged_calib.y_adc_high);
                float y_drift = std::max(fabsf(y_min_normalized), fabsf(y_max_normalized));
                staged_calib.deadzone = Helpers::clamp(DZ_BUFFER * std::max(x_drift, y_drift), 0.0f, DZ_CEILING);
            }
            // simple validation
            if (staged_calib.x_adc_low > staged_calib.x_adc_high)
                Helpers::swap(staged_calib.x_adc_low, staged_calib.x_adc_high);
            if (staged_calib.y_adc_low > staged_calib.y_adc_high)
                Helpers::swap(staged_calib.y_adc_low, staged_calib.y_adc_high);
            staged_calib.x_adc_center = Helpers::clamp(staged_calib.x_adc_center, staged_calib.x_adc_low, staged_calib.x_adc_high);
            staged_calib.y_adc_center = Helpers::clamp(staged_calib.y_adc_center, staged_calib.y_adc_low, staged_calib.y_adc_high);
            if (staged_calib.save(ctx.storage, ctx.logger))
                beginStage(ctx, Done);
            else
                beginStage(ctx, Canceled);
            break;
        default:
            break;
        }
    }
}

// Stage complete: latch the measured value into cur_.
// Left/Up use minima; Right/Down use maxima; Center uses average.
void CalibrationScene::handleDone(AppContext &ctx)
{
    const millis_t elapsed = ctx.time.nowMs() - stage_start_;
    if (elapsed > STAGE_MS || (elapsed > TRANSITION_BUFFER && ctx.input.state().pressed))
    {
        ctx.input.setCalibration(staged_calib);
        state_ = Idle;
    }
}

// Safety: pressing the button at any time cancels and restores base_ values.
// Prevents accidental writes from brief or unintended presses.
void CalibrationScene::handleCanceled(AppContext &ctx)
{
    const millis_t elapsed = ctx.time.nowMs() - stage_start_;
    if (elapsed > STAGE_MS || (elapsed > TRANSITION_BUFFER && ctx.input.state().pressed))
    {
        state_ = Idle;
    }
}

void CalibrationScene::loop(AppContext &ctx)
{
    switch (state_)
    {
    case Idle:
        handleIdle(ctx);
        break;
    case StageLeft:
    case StageRight:
    case StageUp:
    case StageDown:
    case StageCenter:
        handleStage(ctx);
        break;
    case Done:
        handleDone(ctx);
        break;
    case Canceled:
        handleCanceled(ctx);
        break;
    }
}
