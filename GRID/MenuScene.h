#ifndef MENU_SCENE_H
#define MENU_SCENE_H
#include "Scene.h"
#include "SceneBus.h"
#include <array>

struct MenuScene : public Scene
{
    // Replace enum with a static array of menu items
    struct MenuItem
    {
        const char *label;
        std::function<void()> SceneBus::*action; // pointer to SceneBus member std::function
    };

    // items - defined in cpp
    static const std::array<MenuItem, 6> kItems; // Change size as needed

    // index into kItems
    std::size_t selected = 0;

    SceneTimingPrefs timingPrefs() const override
    {
        // Default 60Hz is fine; return NaN to keep global default
        return SceneTimingPrefs(std::numeric_limits<double>::quiet_NaN());
    }

    void setup(AppContext &ctx) override {} // no-op

    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Menu; }
    const char *label() const override { return "Menu"; }

private:
    // Basic nav: X left/right to change selection, button to activate
    // Simple hysteresis with thresholds
    static constexpr float HYSTERESIS_THRESHOLD = 0.45f;
    static const millis_t SELECT_WAIT = 500; // wait after select for drama

    void next();
    void prev();
    void draw(AppContext &ctx, bool left, bool right, bool press);
    // removed label(const Item) - use kItems[selected].label instead
};

#endif
