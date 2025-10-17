#include "MenuScene.h"

void MenuScene::loop(AppContext &ctx)
{
    const InputState s = ctx.input.state();

    // Basic nav: X left/right to change selection, button to activate
    // Simple hysteresis with thresholds
    const float TH = 0.45f;
    static bool prevLeft = false, prevRight = false, prevPress = false;

    bool left = (s.x < -TH);
    bool right = (s.x > TH);
    bool press = s.pressed;

    if (right && !prevRight)
        next();
    if (left && !prevLeft)
        prev();

    // Draw menu each frame
    draw(ctx);

    if (press && !prevPress && ctx.bus)
    {
        switch (selected)
        {
        case Item::Example:
            ctx.bus->toExample();
            return;
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

void MenuScene::draw(AppContext &ctx)
{
    ctx.gfx.clear();
    ctx.gfx.setTextSize(1);
    ctx.gfx.setCursor(1, 1);

    ctx.gfx.print("Menu:");

    auto printItem = [&](const char *label, bool sel)
    {
        ctx.gfx.setTextColor(sel ? GREEN : WHITE);
        ctx.gfx.println(label);
    };

    printItem("E.g.", selected == Item::Example);
    printItem("Boids", selected == Item::Boids);
    printItem("Calib", selected == Item::Calibration);

    // arrow hint
    ctx.gfx.setCursor(1, MATRIX_HEIGHT - 8);
    ctx.gfx.setTextColor(WHITE);
    ctx.gfx.print("<OK>");
}
