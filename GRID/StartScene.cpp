#include "StartScene.h"

void StartScene::setup(AppContext &ctx)
{
    // setup variables, scrollText
    const int scale = 1;
    const int yTop = ScrollText::yTopCentered(scale);
    banner_.prepare(ctx.gfx, "Welcome to the ", scale, Colors::Muted::White, Colors::Black, true, false);
    banner_.reset(MATRIX_WIDTH, std::max(0, yTop));
}

void StartScene::loop(AppContext &ctx)
{
    // 1) show scrollText intro
    banner_.step(ctx.gfx, -1); // scroll to left
    if (banner_.rightEdge() < MATRIX_WIDTH)
        ctx.logger.logf(LogLevel::Debug, "DONE");
    // 2) show GRID glyph
    // 3) pause
    // 4) transition to MenuScene
}
