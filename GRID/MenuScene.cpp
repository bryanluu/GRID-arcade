#include "MenuScene.h"

void MenuScene::loop(AppContext &ctx)
{
    const InputState s = ctx.input.state();

    // Basic nav: X left/right to change selection, button to activate
    static bool prevLeft = false, prevRight = false, prevPress = false;

    bool left = (s.x < -MenuScene::HYSTERESIS_THRESHOLD);
    bool right = (s.x > MenuScene::HYSTERESIS_THRESHOLD);
    bool press = s.pressed;

    if (left && !prevLeft)
        next();
    if (right && !prevRight)
        prev();

    // Draw menu each frame
    draw(ctx, left, right, press);

    if (press && !prevPress && ctx.bus)
    {
        // shows center press with a small pause
        ctx.gfx.show();
        ctx.time.sleep(SELECT_WAIT);
        switch (selected)
        {
        case Item::Boids:
            ctx.bus->toBoids();
            return;
        case Item::Calibration:
            ctx.bus->toCalibration();
            return;
        default:
            break;
        }
    }

    prevLeft = left;
    prevRight = right;
    prevPress = press;
}

void MenuScene::next()
{
    int v = static_cast<int>(selected);
    v = (v + 1) % static_cast<int>(Item::COUNT);
    selected = static_cast<Item>(v);
}

void MenuScene::prev()
{
    int v = static_cast<int>(selected);
    v = (v - 1 + static_cast<int>(Item::COUNT)) % static_cast<int>(Item::COUNT);
    selected = static_cast<Item>(v);
}

void MenuScene::draw(AppContext &ctx, bool left, bool right, bool press)
{
    ctx.gfx.clear();
    ctx.gfx.setTextSize(1);
    ctx.gfx.setCursor(1, 1);
    ctx.gfx.setTextColor(Colors::Muted::White);

    ctx.gfx.print("Menu:");
    ctx.gfx.setCursor(1, 12);
    if (press)
        ctx.gfx.setTextColor(Colors::Bright::Green);
    else
        ctx.gfx.setTextColor(Colors::Muted::White);
    ctx.gfx.println(label(selected));

    // arrow hint
    ctx.gfx.setCursor(1, MATRIX_HEIGHT - 8);
    if (left)
        ctx.gfx.setTextColor(Colors::Bright::White);
    else
        ctx.gfx.setTextColor(Colors::Muted::White);
    ctx.gfx.print("<");

    ctx.gfx.setCursor(10, MATRIX_HEIGHT - 8);
    if (press)
        ctx.gfx.setTextColor(Colors::Bright::White);
    else
        ctx.gfx.setTextColor(Colors::Muted::White);
    ctx.gfx.print("OK");

    if (right)
        ctx.gfx.setTextColor(Colors::Bright::White);
    else
        ctx.gfx.setTextColor(Colors::Muted::White);
    ctx.gfx.setCursor(MATRIX_WIDTH - 6, MATRIX_HEIGHT - 8);
    ctx.gfx.print(">");
}

const char *MenuScene::label(const MenuScene::Item scene) const
{
    switch (scene)
    {
    case MenuScene::Item::Boids:
        return "Boids";
    case MenuScene::Item::Calibration:
        return "Calib";
    default:
        return "NULL";
    }
}
