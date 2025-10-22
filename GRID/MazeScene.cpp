#include "MazeScene.h"

const char *MazeScene::textPlayer = "You";
const char *MazeScene::textStart = "Start";
const char *MazeScene::textWall = "Wall";
const char *MazeScene::textFinish = "Exit";
const char *MazeScene::textHint = "Hint";
const char *MazeScene::textFood = "Food";
const char *MazeScene::textTime = "Time";

const Color333 MazeScene::kPlayerColor = Colors::Muted::White;
const Color333 MazeScene::kWallColor = Colors::Muted::Red;
const Color333 MazeScene::kStartColor = Colors::Muted::Blue;
const Color333 MazeScene::kFinishColor = Colors::Muted::Green;
const Color333 MazeScene::kSolutionColor = Colors::Muted::Yellow;
const Color333 MazeScene::kFoodColor = Colors::Muted::Cyan;
const Color333 MazeScene::kTimeColor = Colors::Muted::Violet;

// ########## START CODE ##########

/**
 * @brief Displays the starting graphics
 */
void MazeScene::renderHints(AppContext &ctx)
{
    ctx.gfx.clear();
    ctx.gfx.setCursor(0, textY--);
    ctx.gfx.setTextColor(kPlayerColor);
    ctx.gfx.println(textPlayer);
    ctx.gfx.setTextColor(kWallColor);
    ctx.gfx.println(textWall);
    ctx.gfx.setTextColor(kStartColor);
    ctx.gfx.println(textStart);
    ctx.gfx.setTextColor(kFinishColor);
    ctx.gfx.println(textFinish);
    ctx.gfx.setTextColor(kSolutionColor);
    ctx.gfx.println(textHint);
    ctx.gfx.setTextColor(kFoodColor);
    ctx.gfx.println(textFood);
    ctx.gfx.setTextColor(kTimeColor);
    ctx.gfx.println(textTime);
}

void MazeScene::setup(AppContext &ctx)
{
    startTime = ctx.time.nowMs();
    textY = 5;

    // show Hint text before game starts
    ctx.gfx.setImmediate(false);
    while (textY > -kStartLoopSteps)
    {
        renderHints(ctx);
        ctx.gfx.show();
        ctx.time.sleep(kStartLoopDelay);
    }
    ctx.gfx.setImmediate(true);
}

void MazeScene::loop(AppContext &ctx)
{
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.println("MAZE");
}
