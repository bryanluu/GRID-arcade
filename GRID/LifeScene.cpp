#include "LifeScene.h"
#include "Colors.h"

const Color333 LifeScene::ALIVE_COLOR = Colors::Muted::White;
const Color333 LifeScene::DEAD_COLOR = Colors::Black;
const Color333 LifeScene::CURSOR_ACTIVE_COLOR = Colors::Bright::Green;
const Color333 LifeScene::CURSOR_IDLE_COLOR = Colors::Muted::Green;

void LifeScene::setup(AppContext &ctx)
{
}

void LifeScene::loop(AppContext &ctx)
{
    ctx.gfx.clear();
    drawCells(ctx.gfx);
    if (!started)
    {
        updateCursor(ctx);
        drawCursor(ctx);
    }
}

void LifeScene::drawCells(Matrix32 &gfx)
{
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            int idx = index(x, y);
            if (cells.test(idx))
            {
                gfx.setSafe(x, y, ALIVE_COLOR);
            }
            else
            {
                gfx.setSafe(x, y, DEAD_COLOR);
            }
        }
    }
}

void LifeScene::updateCursor(AppContext &ctx)
{
    InputState input = ctx.input.state();
    // Move cursor with D-pad / joystick
    if (input.x < -0.5f)
    {
        cursorX = std::max(0, cursorX - 1);
    }
    else if (input.x > 0.5f)
    {
        cursorX = std::min(MATRIX_WIDTH - 1, cursorX + 1);
    }
    if (input.y < -0.5f)
    {
        cursorY = std::max(0, cursorY - 1);
    }
    else if (input.y > 0.5f)
    {
        cursorY = std::min(MATRIX_HEIGHT - 1, cursorY + 1);
    }

    // Click spawns a cell, long-press starts the simulation
    if (input.pressed)
    {
        if (lastPressTime == 0) // rising edge -- toggle cell
        {
            lastPressTime = ctx.time.nowMs();
            int cursorIdx = index(cursorX, cursorY);
            cells.flip(cursorIdx);
        }
        if (!started && (ctx.time.nowMs() - lastPressTime) >= TRIGGER_WAIT)
        {
            started = true; // long press starts simulation
        }
    }
    else
    {
        lastPressTime = 0; // reset on release
    }
}

void LifeScene::drawCursor(AppContext &ctx)
{
    Color333 cursorColor = ctx.input.state().pressed ? CURSOR_ACTIVE_COLOR : CURSOR_IDLE_COLOR;
    ctx.gfx.setSafe(cursorX, cursorY, cursorColor);
}
