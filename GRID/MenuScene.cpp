#include "MenuScene.h"

// Define the static item array (labels + pointer-to-member in SceneBus)
const std::array<MenuScene::MenuItem, 6> MenuScene::kItems = {
    MenuScene::MenuItem{"Snake", &SceneBus::toSnake},
    MenuScene::MenuItem{"Life", &SceneBus::toLife},
    MenuScene::MenuItem{"Maze", &SceneBus::toMaze},
    MenuScene::MenuItem{"Boids", &SceneBus::toBoids},
    MenuScene::MenuItem{"Calib", &SceneBus::toCalibration},
    MenuScene::MenuItem{"QR", &SceneBus::toQR}};

void MenuScene::loop(AppContext &ctx)
{
    const InputState s = ctx.input.state();

    // Basic nav: X left/right to change selection, button to activate
    static bool prevLeft = false, prevRight = false, prevPress = false;

    bool left = (s.x < -MenuScene::HYSTERESIS_THRESHOLD);
    bool right = (s.x > MenuScene::HYSTERESIS_THRESHOLD);
    bool press = s.pressed;

    if (left && !prevLeft)
        prev();
    if (right && !prevRight)
        next();

    // Draw menu each frame
    draw(ctx, left, right, press);

    if (press && !prevPress && ctx.bus)
    {
        // shows center press with a small pause
        ctx.gfx.show();
        ctx.time.sleep(SELECT_WAIT);

        // Call the stored SceneBus action if present
        auto actionMemberPtr = MenuScene::kItems[selected].action;
        if (ctx.bus && actionMemberPtr)
        {
            std::function<void()> &actionCallback = ctx.bus->*actionMemberPtr;
            if (actionCallback)
            {
                actionCallback();
                return;
            }
        }
    }

    prevLeft = left;
    prevRight = right;
    prevPress = press;
}

void MenuScene::next()
{
    selected = (selected + 1) % kItems.size();
}

void MenuScene::prev()
{
    selected = (selected + kItems.size() - 1) % kItems.size();
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

    ctx.gfx.println(kItems[selected].label);

    // arrow hint
    ctx.gfx.setCursor(1, MATRIX_HEIGHT - 8);
    ctx.gfx.setTextColor(left ? Colors::Bright::White : Colors::Muted::White);
    ctx.gfx.print("<");

    ctx.gfx.setCursor(10, MATRIX_HEIGHT - 8);
    ctx.gfx.setTextColor(press ? Colors::Bright::White : Colors::Muted::White);
    ctx.gfx.print("OK");

    ctx.gfx.setCursor(MATRIX_WIDTH - 6, MATRIX_HEIGHT - 8);
    ctx.gfx.setTextColor(right ? Colors::Bright::White : Colors::Muted::White);
    ctx.gfx.print(">");
}
