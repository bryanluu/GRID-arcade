#include "MazeScene.h"

const char *Maze::StartScene::textPlayer = "You";
const char *Maze::StartScene::textStart = "Start";
const char *Maze::StartScene::textWall = "Wall";
const char *Maze::StartScene::textFinish = "Exit";
const char *Maze::StartScene::textHint = "Hint";
const char *Maze::StartScene::textFood = "Food";
const char *Maze::StartScene::textTime = "Time";

// ########## START CODE ##########

/**
 * @brief Displays the starting graphics
 */
void Maze::StartScene::renderHints(AppContext &ctx)
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

void Maze::StartScene::setup(AppContext &ctx)
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

void Maze::StartScene::loop(AppContext &ctx)
{
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.println("MAZE");
}
