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
    else
    {
        if (ctx.time.nowMs() - lastUpdateTime >= kUpdateDelayMs)
        {
            updateCells();
            lastUpdateTime = ctx.time.nowMs();
        }
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

void LifeScene::updateCells()
{
    std::bitset<MATRIX_WIDTH * MATRIX_HEIGHT> newCells;
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            int idx = index(x, y);
            int liveNeighbors = 0;
            // Count live neighbors
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    if (dx == 0 && dy == 0)
                        continue; // skip self
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < MATRIX_WIDTH && ny >= 0 && ny < MATRIX_HEIGHT)
                    {
                        int nIdx = index(nx, ny);
                        if (cells.test(nIdx))
                            liveNeighbors++;
                    }
                }
            }
            // Apply Game of Life rules
            if (cells.test(idx))
            {
                // Cell is alive
                if (liveNeighbors == 2 || liveNeighbors == 3)
                    newCells.set(idx); // stays alive
                else
                    newCells.reset(idx); // dies
            }
            else
            {
                // Cell is dead
                if (liveNeighbors == 3)
                    newCells.set(idx); // becomes alive
                else
                    newCells.reset(idx); // stays dead
            }
        }
    }
    cells = newCells;
}

// --- new helper copied/adapted from MazeScene ---
LifeScene::Direction LifeScene::sampleStrobedDirection(AppContext &ctx, Direction &ioDir, millis_t &ioLastTimeMs)
{
    InputState s = ctx.input.state();
    millis_t nowMs = ctx.time.nowMs();
    // Reset instantaneous direction
    ioDir = Direction::DNone;

    const float dx = s.x;
    const float dy = s.y;

    // If outside deadband, consider a move on the dominant axis
    if (std::fabs(dx) > kInputBuffer || std::fabs(dy) > kInputBuffer)
    {
        // Default repeat delay, faster near extremes
        millis_t inputDelay = kDefaultInputDelayMs;

        // Decide axis and candidate direction
        Direction cand = Direction::DNone;
        if (std::fabs(dx) >= std::fabs(dy))
        {
            if (dx > kInputBuffer)
                cand = Direction::DRight;
            if (dx < -kInputBuffer)
                cand = Direction::DLeft;
            if (std::fabs(dx) >= kFastInputThreshold)
                inputDelay = kFastInputDelayMs;
        }
        else
        {
            if (dy > kInputBuffer)
                cand = Direction::DDown;
            if (dy < -kInputBuffer)
                cand = Direction::DUp;
            if (std::fabs(dy) >= kFastInputThreshold)
                inputDelay = kFastInputDelayMs;
        }

        // Gate by repeat timer (like currentTime - lastInputTime > inputDelay)
        if (cand != Direction::DNone && (nowMs - ioLastTimeMs) > inputDelay)
        {
            ioDir = cand;
        }
    }

    // Update last time only when a step will be consumed this frame
    if (ioDir != Direction::DNone)
        ioLastTimeMs = nowMs;

    return ioDir;
}

void LifeScene::updateCursor(AppContext &ctx)
{
    // sample strobed direction (updates inputDir + lastMoveTime)
    sampleStrobedDirection(ctx, inputDir, lastMoveTime);

    // Move cursor when a strobed direction is produced
    if (inputDir != Direction::DNone)
    {
        switch (inputDir)
        {
        case Direction::DLeft:
            cursorX = std::max(0, cursorX - 1);
            break;
        case Direction::DRight:
            cursorX = std::min(MATRIX_WIDTH - 1, cursorX + 1);
            break;
        case Direction::DUp:
            cursorY = std::max(0, cursorY - 1);
            break;
        case Direction::DDown:
            cursorY = std::min(MATRIX_HEIGHT - 1, cursorY + 1);
            break;
        default:
            break;
        }
    }

    // Click spawns a cell, long-press starts the simulation
    if (ctx.input.state().pressed)
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
