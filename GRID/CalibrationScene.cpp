#include "CalibrationScene.h"
#include "Colors.h"
#include <cstring>

constexpr Color333 CalibrationScene::outlineColor;
constexpr Color333 CalibrationScene::cursorColor;
constexpr Color333 CalibrationScene::pressedColor;
const char CalibrationScene::message[33] = "Press for 2 seconds to calibrate";

void CalibrationScene::setup(AppContext &ctx)
{
    ctx.gfx.setImmediate(false); // one present per frame
    ctx.gfx.setTextSize(1);
    ctx.gfx.setTextColor(WHITE);

    static std::vector<PixelMap> msgCols;
    ctx.gfx.buildStringCols(message, msgCols);
    const int ts = 1; // using setTextSize(1)
    const int totalCols = int(msgCols.size());
    const int screenCols = MATRIX_WIDTH / ts;
    const int y = 0;

    // Optional: dim background once
    ctx.gfx.clear();
    ctx.gfx.show();

    // Scroll from right to left
    for (int head = MATRIX_WIDTH; head > -totalCols * ts; --head)
    {
        // Erase only the vacated 1-column strip behind the text
        // Compute previous and current visible windows
        // Simpler: redraw only the text region (still cheap with spans)
        // Clear a minimal band where text is drawn
        ctx.gfx.fillRect(0, y, MATRIX_WIDTH, FONT_GLYPH_HEIGHT * ts, BLACK);

        // Determine which msg columns are visible
        int firstCol = std::max(0, (-head + (ts - 1)) / ts);
        int lastCol = std::min(totalCols, (MATRIX_WIDTH - head + ts - 1) / ts);
        if (firstCol < lastCol)
        {
            // x position where firstCol lands
            int x0 = head + firstCol * ts;
            ctx.gfx.blitCols(x0, y, msgCols.data() + firstCol, lastCol - firstCol, WHITE, ts);
        }
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
