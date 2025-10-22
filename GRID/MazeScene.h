#ifndef MAZE_SCENE_H
#define MAZE_SCENE_H

#include "Scene.h"
#include "Colors.h"

namespace Maze
{
    static const Color333 kPlayerColor = Colors::Muted::White;
    static const Color333 kWallColor = Colors::Muted::Red;
    static const Color333 kStartColor = Colors::Muted::Blue;
    static const Color333 kFinishColor = Colors::Muted::Green;
    static const Color333 kSolutionColor = Colors::Muted::Yellow;
    static const Color333 kFoodColor = Colors::Muted::Cyan;
    static const Color333 kTimeColor = Colors::Muted::Violet;

    class StartScene : public Scene
    {
    private:
        // Start Scene config
        static const byte kStartLoopSteps = 55;      // cycles to wait for
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
};

#endif // MAZE_SCENE_H
