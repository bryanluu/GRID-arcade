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

public:
    SceneKind kind() const override { return SceneKind::Life; }
    const char *label() const override { return "Life"; }
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // LIFE_SCENE_H
