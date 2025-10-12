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
    static const char message[35] = "Press for 2 seconds to calibrate  ";
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

void CalibrationScene::loop(AppContext &ctx)
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
