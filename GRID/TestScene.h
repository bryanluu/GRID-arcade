#ifndef TEST_SCENE_H
#define TEST_SCENE_H

#include "Scene.h"
#include "AppContext.h"
#include "IStorage.h"
#include "Matrix32.h"
#include "Logging.h"

class TestScene : public Scene
{
public:
    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override {}; // no-op

    SceneKind kind() const override { return SceneKind::Test; }
    const char *label() const override { return "Test"; }

private:
    void test_hue_sweep(Matrix32 &m);
    void test_brightness_ramp(Matrix32 &m);
    void test_primary_blocks(Matrix32 &m);
    void test_saturation_stripes(Matrix32 &m);
    void run_color_tests(AppContext &ctx);
    void run_storage_smoke(IStorage &storage, ILogger &logger);
};

#endif // TEST_SCENE_H
