#include "CalibrationScene.h"
#include "Colors.h"
#include <cstring>

constexpr Color333 CalibrationScene::outlineColor;
constexpr Color333 CalibrationScene::cursorColor;
constexpr Color333 CalibrationScene::pressedColor;
const char CalibrationScene::message[33] = "Press for 2 seconds to calibrate";

void CalibrationScene::setup(AppContext &ctx)
{
    int msg_len = strlen(message);
    ctx.gfx.setTextSize(1);

    // scroll text
    for (int i = MATRIX_WIDTH; i > -msg_len * FONT_CHAR_WIDTH; i--)
    {
        ctx.gfx.setCursor(i, 7);
        ctx.gfx.clear();
        ctx.gfx.print(message);
        ctx.gfx.show();
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
