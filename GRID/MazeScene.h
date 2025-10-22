#ifndef MAZE_SCENE_H
#define MAZE_SCENE_H

#include "Scene.h"
#include "Colors.h"

class MazeScene : public Scene
{
private:
    // Color Config
    static const Color333 kPlayerColor;
    static const Color333 kWallColor;
    static const Color333 kStartColor;
    static const Color333 kFinishColor;
    static const Color333 kSolutionColor;
    static const Color333 kFoodColor;
    static const Color333 kTimeColor;
    // Start Scene config
    static const int8_t kStartLoopStopY = -60;   // vertical position to stop hints
    static const millis_t kStartLoopDelay = 150; // in ms
    static const char *textPlayer;
    static const char *textStart;
    static const char *textWall;
    static const char *textFinish;
    static const char *textHint;
    static const char *textFood;
    static const char *textTime;
    millis_t startTime = 0;
    int16_t textY;

    void renderHints(AppContext &ctx);

public:
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Maze; }
    const char *label() const override { return "Maze"; }
};

#endif // MAZE_SCENE_H
