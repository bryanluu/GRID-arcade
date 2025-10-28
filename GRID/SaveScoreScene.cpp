#include "SaveScoreScene.h"
#include "SceneBus.h"

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
    startTime_ = ctx.time.nowMs();
    stage_ = newStage;
}

void SaveScoreScene::setup(AppContext &ctx)
{
    setStage(ctx, Stage::ShowIntro);
}

void SaveScoreScene::loop(AppContext &ctx)
{
    millis_t now = ctx.time.nowMs();
    switch (stage_)
    {
    case Stage::ShowIntro:
        if (now - startTime_ < kShowTextDuration)
            showIntro(ctx);
        else
            setStage(ctx, Stage::InputName);
        break;
    case Stage::InputName:
    {
        InputState state = ctx.input.state();
        if (state.pressed) // submit name once button is pressed
        {
            // TODO implement submit storage
            setStage(ctx, Stage::ShowSaved);
        }
        else
        {
            handleInputName(ctx);
        }
        break;
    }
    case Stage::ShowSaved:
        if (now - startTime_ < kShowTextDuration)
            showSaved(ctx);
        else
            setStage(ctx, Stage::End);
        break;
    case Stage::End:
        ctx.bus->toMenu();
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

void SaveScoreScene::showSaved(AppContext &ctx)
{
    int ts = 1;
    ctx.gfx.clear();
    ctx.gfx.setTextSize(ts);
    ctx.gfx.setTextColor(Colors::Muted::White);
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.println("Score");
    ctx.gfx.println("Saved");
}

void SaveScoreScene::handleInputName(AppContext &ctx)
{
    ctx.gfx.clear(); // TODO implement input name
}
