#include "BonnehScene.h"

void BonnehScene::setup(AppContext &ctx)
{
    ctx.gfx.setSafe(kCenterX, kCenterY, kCenterDotColor);
    ctx.gfx.setSafe(kDistractorBottomX, kDistractorBottomY, kDistractorColor);
    ctx.gfx.setSafe(kDistractorTopLeftX, kDistractorTopLeftY, kDistractorColor);
    ctx.gfx.setSafe(kDistractorTopRightX, kDistractorTopRightY, kDistractorColor);
}

void BonnehScene::loop(AppContext &ctx)
{
}
