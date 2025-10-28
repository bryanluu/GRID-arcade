#include "SaveScoreScene.h"

void SaveScoreScene::setStage(AppContext &ctx, Stage newStage)
{
    switch (newStage)
    {
    case Stage::ShowIntro:

        break;
    case Stage::InputName:

        break;
    case Stage::ShowSaved:

        break;
    case Stage::End:

        break;
    }
    startTime = ctx.time.nowMs();
    stage = newStage;
}

void SaveScoreScene::setup(AppContext &ctx)
{
    setStage(ctx, Stage::ShowIntro);
}

void SaveScoreScene::loop(AppContext &ctx)
{
    millis_t now = ctx.time.nowMs();
    switch (stage)
    {
    case Stage::ShowIntro:
        if (now - startTime < kShowTextDuration)
            showIntro(ctx);
        else
            setStage(ctx, Stage::InputName);
        break;
    case Stage::InputName:
        ctx.gfx.clear(); // TODO implement name input
        break;

    case Stage::ShowSaved:

        break;
    case Stage::End:

        break;
    }
}

void SaveScoreScene::showIntro(AppContext &ctx)
{
    int ts = 1;
    ctx.gfx.clear();
    ctx.gfx.setTextSize(ts);
    ctx.gfx.setTextColor(Colors::Muted::White);
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.println("Input");
    ctx.gfx.println("Your");
    ctx.gfx.println("Name");
}
