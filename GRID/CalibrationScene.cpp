#include "CalibrationScene.h"
#include "Colors.h"
#include "ScrollTextHelper.h"
#include <cstring>

constexpr Color333 CalibrationScene::outlineColor;
constexpr Color333 CalibrationScene::cursorColor;
constexpr Color333 CalibrationScene::pressedColor;
const char CalibrationScene::message[33] = "Press for 2 seconds to calibrate";

void CalibrationScene::setup(AppContext &ctx)
{
    ctx.gfx.setImmediate(false);
    ctx.gfx.setTextSize(1);

    ScrollText banner;
    banner.prepare(ctx.gfx, message, /*scale=*/1, WHITE);
    banner.reset(/*startX=*/MATRIX_WIDTH, /*yTop=*/4);

    // Smooth scroll at 1 px per frame
    while (!banner.step(ctx.gfx, /*dx=*/-1))
    {
        ctx.time.sleep(20);
    }
}

void CalibrationScene::loop(AppContext &ctx)
{
    ctx.gfx.clear();

    // draw a circle for possible joystick output range
    ctx.gfx.drawCircle(circleCenter, circleCenter, circleRadius, GRAY);

    // draw the axis cross
    ctx.gfx.drawLine(0, circleRadius, 2 * circleRadius, circleRadius, GRAY);
    ctx.gfx.drawLine(circleRadius, 0, circleRadius, 2 * circleRadius, GRAY);

    // draw the cursor
    InputState state = ctx.input.state();
    int cursorX = round(circleCenter + state.x * circleRadius);
    int cursorY = round(circleCenter + state.y * circleRadius);
    ctx.gfx.drawPixel(cursorX, cursorY, state.pressed ? pressedColor : cursorColor);
}
