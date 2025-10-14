#include "CalibrationScene.h"
#include "Colors.h"
#include "ScrollTextHelper.h"
#include <cstring>

constexpr Color333 CalibrationScene::CIRCLE_COLOR;
constexpr Color333 CalibrationScene::PRESSED_COLOR;
constexpr Color333 CalibrationScene::IDLE_COLOR;
constexpr Color333 CalibrationScene::PRESSED_CURSOR_COLOR;
constexpr Color333 CalibrationScene::IDLE_CURSOR_COLOR;

void CalibrationScene::setup(AppContext &ctx)
{
    static const char message[35] = "Press for 3 seconds to calibrate  ";
    ctx.gfx.setImmediate(false);
    int ts = 1;
    ctx.gfx.setTextSize(ts);

    ScrollText banner;
    banner.prepare(ctx.gfx, message, /*scale=*/ts, WHITE);
    banner.reset(/*startX=*/MATRIX_WIDTH, /*yTop=*/banner.yTopCentered(ts));

    // Smooth scroll at 1 px per frame
    while (!banner.step(ctx.gfx, /*dx=*/-1))
    {
        ctx.time.sleep(20);
    }
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
        // TODO implement
        // draw_progress(clamp01(held / float(HOLD_TO_START_MS)), "Hold 2s to start");
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
    cx_acc_ = cy_acc_ = 0;
    c_count_ = 0;
    ctx.gfx.clear();
    ctx.gfx.setCursor(1, 1);

    switch (s)
    {
    case Done:
        ctx.logger.logf(LogLevel::Info, "Calibration complete");
        ctx.gfx.println("Calib");
        ctx.gfx.setCursor(1, 10);
        ctx.gfx.print("Save");
        break;
    case Canceled:
        ctx.logger.logf(LogLevel::Info, "Calibration canceled");
        ctx.gfx.println("Calib");
        ctx.gfx.setCursor(1, 10);
        ctx.gfx.print("Stop");
        break;
    default:
        ctx.logger.logf(LogLevel::Info, "Calibration Stage: %s", stageLabel(state_));
        ctx.gfx.println("Keep");
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
    ctx.gfx.fillRect(BAR_START_X, BAR_START_Y, BAR_START_X + BAR_WIDTH, BAR_START_Y + BAR_HEIGHT, BLACK);
    ctx.gfx.fillRect(BAR_START_X, BAR_START_Y, elapsed_width, BAR_START_Y + BAR_HEIGHT, BAR_COLOR);
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
    cx_acc_ += ax;
    cy_acc_ += ay;
    ++c_count_;

    // const float t = Helpers::clamp(elapsed / float(STAGE_MS), 0.0f, 1.0f);
    // draw_progress(t, stageLabel(state_)); // TODO implement
    drawStage(ctx);
    if (elapsed >= TRANSITION_BUFFER + STAGE_MS)
    {
        switch (state_)
        {
        case StageLeft:
            cur_.x_adc_low = Helpers::clamp(x_min_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageRight);
            break;
        case StageRight:
            cur_.x_adc_high = Helpers::clamp(x_max_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageUp);
            break;
        case StageUp:
            cur_.y_adc_low = Helpers::clamp(y_min_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageDown);
            break;
        case StageDown:
            cur_.y_adc_high = Helpers::clamp(y_max_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            beginStage(ctx, StageCenter);
            break;
        case StageCenter:
            if (c_count_ > 0)
            {
                cur_.x_adc_center = Helpers::clamp(cx_acc_ / c_count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
                cur_.y_adc_center = Helpers::clamp(cy_acc_ / c_count_, InputCalibration::ADC_MIN, InputCalibration::ADC_MAX);
            }
            // simple validation
            if (cur_.x_adc_low > cur_.x_adc_high)
                Helpers::swap(cur_.x_adc_low, cur_.x_adc_high);
            if (cur_.y_adc_low > cur_.y_adc_high)
                Helpers::swap(cur_.y_adc_low, cur_.y_adc_high);
            cur_.x_adc_center = Helpers::clamp(cur_.x_adc_center, cur_.x_adc_low, cur_.x_adc_high);
            cur_.y_adc_center = Helpers::clamp(cur_.y_adc_center, cur_.y_adc_low, cur_.y_adc_high);
            beginStage(ctx, Done);
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
        cur_ = base_; // discard edits}
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
