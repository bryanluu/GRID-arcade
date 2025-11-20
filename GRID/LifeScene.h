#ifndef LIFE_SCENE_H
#define LIFE_SCENE_H

#include "Scene.h"
#include <bitset>

class LifeScene final : public Scene
{
    static const Color333 ALIVE_COLOR;
    static const Color333 DEAD_COLOR;
    static const Color333 CURSOR_ACTIVE_COLOR;
    static const Color333 CURSOR_IDLE_COLOR;
    static constexpr millis_t TRIGGER_WAIT = 3000;

    int cursorX = (MATRIX_WIDTH / 2);
    int cursorY = (MATRIX_HEIGHT / 2);
    bool started = false;
    millis_t lastPressTime = 0;
    std::bitset<MATRIX_WIDTH * MATRIX_HEIGHT> cells;

    int index(int x, int y) const
    {
        return y * MATRIX_WIDTH + x;
    }

    void drawCells(Matrix32 &gfx);
    void updateCursor(AppContext &ctx);
    void drawCursor(AppContext &ctx);
    // void updateCells();

    // --- strobed input members (copied/adapted from MazeScene) ---
    enum Direction : int8_t
    {
        DNone = -1,
        DRight,
        DDown,
        DLeft,
        DUp
    };
    Direction inputDir = Direction::DNone;
    static constexpr float kInputBuffer = 0.15f;
    static constexpr float kFastInputThreshold = 0.90f;
    static constexpr millis_t kDefaultInputDelayMs = 150;
    static constexpr millis_t kFastInputDelayMs = 60;
    millis_t lastMoveTime = 0;

    // helper used by updateCursor to sample strobed direction
    Direction sampleStrobedDirection(AppContext &ctx, Direction &ioDir, millis_t &ioLastTimeMs);

public:
    SceneKind kind() const override { return SceneKind::Life; }
    const char *label() const override { return "Life"; }
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // LIFE_SCENE_H
