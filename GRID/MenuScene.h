#ifndef MENU_SCENE_H
#define MENU_SCENE_H
#include "Scene.h"
#include "SceneBus.h"
#include <array>

struct MenuScene : public Scene
{
    enum class Item : int
    {
        Example,
        Boids,
        Calibration,
        COUNT
    };

    Item selected = Item::Example;

    SceneTimingPrefs timingPrefs() const override
    {
        // Default 60Hz is fine; return NaN to keep global default
        return SceneTimingPrefs(std::numeric_limits<double>::quiet_NaN());
    }

    void setup(AppContext &ctx) override
    {
        // Simple splash
        ctx.gfx.clear();
        ctx.gfx.setCursor(1, 0);
        ctx.gfx.setTextSize(1);
        ctx.gfx.setTextColor(WHITE);
        ctx.gfx.println("Menu");
    }

    void loop(AppContext &ctx) override
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

private:
    void next()
    {
        int v = static_cast<int>(selected);
        v = (v + 1) % static_cast<int>(Item::COUNT);
        selected = static_cast<Item>(v);
    }
    void prev()
    {
        int v = static_cast<int>(selected);
        v = (v - 1 + static_cast<int>(Item::COUNT)) % static_cast<int>(Item::COUNT);
        selected = static_cast<Item>(v);
    }
    void draw(AppContext &ctx)
    {
        ctx.gfx.clear();
        ctx.gfx.setTextSize(1);
        ctx.gfx.setCursor(2, 4);

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
};

#endif
