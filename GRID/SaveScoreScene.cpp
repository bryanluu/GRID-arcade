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
    InputState s = ctx.input.state();

    // Basic nav: X left/right to change cursor
    static bool prevLeft = false, prevRight = false;

    bool left = (s.x < -HYSTERESIS_THRESHOLD);
    bool right = (s.x > HYSTERESIS_THRESHOLD);

    if (left && !prevLeft)
        moveLeft();
    if (right && !prevRight)
        moveRight();

    drawName(ctx);

    prevLeft = left;
    prevRight = right;
}

void SaveScoreScene::moveLeft()
{
    if (cursorIndex_ > 0)
        --cursorIndex_;
}

void SaveScoreScene::moveRight()
{
    if (cursorIndex_ < payload_.kMaxNameLength - 1)
        ++cursorIndex_;
}

void SaveScoreScene::drawName(AppContext &ctx)
{
    int ts = 1;
    int tStartX = 1;
    int tStartY = 12;
    ctx.gfx.clear();
    ctx.gfx.setTextSize(ts);
    ctx.gfx.setTextColor(Colors::Muted::White);
    for (int i = 0; i < payload_.kMaxNameLength; ++i)
    {
        int tx = i * FONT_CHAR_WIDTH + tStartX;
        int ty = tStartY;
        Color333 tc;
        if (i == cursorIndex_)
        {
            tc = kSelectedColor;
            drawCarets(ctx, tx, ty);
        }
        else
        {
            tc = kTextColor;
        }
        ctx.gfx.drawChar(tx, ty, payload_.name[i], tc);
    }
}

void SaveScoreScene::drawCarets(AppContext &ctx, int tx, int ty)
{
    InputState s = ctx.input.state();
    Color333 tc;

    // Basic nav: X up/down to change char
    static bool prevUp = false, prevDown = false;

    bool up = (s.y < -HYSTERESIS_THRESHOLD);
    bool down = (s.y > HYSTERESIS_THRESHOLD);

    // Draw up caret (assuming textSize = 1)
    tc = (up ? kSelectedColor : kTextColor);
    ctx.gfx.drawLine(tx, ty - 2, tx + 2, ty - 4, tc);
    ctx.gfx.drawLine(tx + 3, ty - 3, tx + 4, ty - 2, tc);

    // Draw down caret (assuming textSize = 1)
    tc = (down ? kSelectedColor : kTextColor);
    ctx.gfx.drawLine(tx, ty + FONT_CHAR_HEIGHT + 1, tx + 2, ty + FONT_CHAR_HEIGHT + 3, tc);
    ctx.gfx.drawLine(tx + 3, ty + FONT_CHAR_HEIGHT + 2, tx + 4, ty + FONT_CHAR_HEIGHT + 1, tc);

    prevUp = up;
    prevDown = down;
}
