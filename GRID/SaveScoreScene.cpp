#include "SaveScoreScene.h"
#include "SceneBus.h"

constexpr Color333 SaveScoreScene::kTextColor;
constexpr Color333 SaveScoreScene::kSelectedColor;
constexpr Color333 SaveScoreScene::kSubmitColor;
constexpr Color333 SaveScoreScene::kErrorColor;
constexpr char SaveScoreScene::kAlphabet[];

void SaveScoreScene::setStage(AppContext &ctx, Stage newStage)
{
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
        if (submit_) // submit name once button is pressed
        {
            if (submitScoreData(ctx, payload_))
            {
                setStage(ctx, Stage::ShowSaved);
            }
            else
                setStage(ctx, Stage::ShowError);
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
    case Stage::ShowError:
        if (now - startTime_ < kShowTextDuration)
            showError(ctx);
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
    ctx.gfx.setTextColor(kTextColor);
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
    ctx.gfx.setTextColor(kTextColor);
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.println("Score");
    ctx.gfx.setTextColor(kSubmitColor);
    ctx.gfx.println("Saved");
}

void SaveScoreScene::showError(AppContext &ctx)
{
    int ts = 1;
    ctx.gfx.clear();
    ctx.gfx.setTextSize(ts);
    ctx.gfx.setTextColor(kTextColor);
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.println("Score");
    ctx.gfx.println("Save");
    ctx.gfx.setTextColor(kErrorColor);
    ctx.gfx.println("Error");
}

void SaveScoreScene::handleInputName(AppContext &ctx)
{
    InputState s = ctx.input.state();

    // Basic nav: X left/right to change cursor
    static bool prevLeft = false, prevRight = false;

    bool left = (s.x < -kHysteresisThreshold);
    bool right = (s.x > kHysteresisThreshold);

    if (left && !prevLeft)
        moveLeft();
    if (right && !prevRight)
        moveRight();

    drawName(ctx);

    if (submit_)
    {
        ctx.gfx.show();
        ctx.time.sleep(kSubmitPause);
    }

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
        if (ctx.input.state().pressed)
        {
            tc = kSubmitColor;
            submit_ = true;
        }
        ctx.gfx.drawChar(tx, ty, payload_.name[i], tc);
    }
}

void SaveScoreScene::drawCarets(AppContext &ctx, int tx, int ty)
{
    InputState s = ctx.input.state();
    Color333 tc;
    millis_t now = ctx.time.nowMs();
    // get current character index in alphabet
    alphabetIndex_ = (strchr(kAlphabet, payload_.name[cursorIndex_]) - kAlphabet);

    // Basic nav: X up/down to change char
    static bool prevUp = 0, prevDown = 0;
    static millis_t prevUpTime = 0, prevDownTime = 0;

    bool up = (s.y < -kHysteresisThreshold);
    bool down = (s.y > kHysteresisThreshold);

    if (up && (!prevUp || (now - prevUpTime > kAlphabetStrobeDelay)))
    {
        moveUp();
        prevUpTime = now;
    }
    if (down && (!prevDown || (now - prevDownTime > kAlphabetStrobeDelay)))
    {
        moveDown();
        prevDownTime = now;
    }

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

void SaveScoreScene::moveUp()
{
    alphabetIndex_ = (alphabetIndex_ - 1 + kAlphabetSize) % kAlphabetSize;
    payload_.name[cursorIndex_] = kAlphabet[alphabetIndex_];
}

void SaveScoreScene::moveDown()
{
    alphabetIndex_ = (alphabetIndex_ + 1) % kAlphabetSize;
    payload_.name[cursorIndex_] = kAlphabet[alphabetIndex_];
}

bool SaveScoreScene::submitScoreData(AppContext &ctx, ScoreData &payload)
{
    char filename[10];
    snprintf(filename, 10, "%s.%s", originLabel_, payload.kFileExtension);
    return payload.save(ctx.storage, ctx.logger, filename);
}
