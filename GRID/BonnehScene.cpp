#include "BonnehScene.h"
#include "Matrix32.h"
#include <cmath>

// Out-of-class definitions required pre-C++17 for static constexpr
// members of non-integral type (e.g. Color333) that get odr-used
// (passed by value into setSafe()). Without these, the linker has
// no storage to reference.
constexpr Color333 BonnehScene::kCenterDotColor;
constexpr Color333 BonnehScene::kDistractorColor;
constexpr Color333 BonnehScene::kCrossColor;

void BonnehScene::drawRotatedLattice(AppContext &ctx) const {
    const float rad = rotationDeg_ * static_cast<float>(M_PI) / 180.0f;
    const float cosA = std::cos(rad);
    const float sinA = std::sin(rad);

    // Every cross in the lattice shares the same rotation angle this
    // frame, so its 4 arm offsets only need to be derived once (not
    // recomputed with fresh sin/cos calls per lattice point). The
    // other 3 arms are just 90-degree axis swaps of the first.
    const int arm0Dx = static_cast<int>(std::lround(kCrossArmLength * cosA));
    const int arm0Dy = static_cast<int>(std::lround(kCrossArmLength * sinA));
    const int armDx[4] = {arm0Dx, -arm0Dy, -arm0Dx, arm0Dy};
    const int armDy[4] = {arm0Dy, arm0Dx, -arm0Dy, -arm0Dx};

    for (int j = -kLatticeRadius; j <= kLatticeRadius; ++j) {
        for (int i = -kLatticeRadius; i <= kLatticeRadius; ++i) {
            // Base lattice offset from center (unrotated)
            const float baseX = i * kCrossSpacing;
            const float baseY = j * kCrossSpacing;

            // Rotate the offset rigidly around the center
            const float rotX = baseX * cosA - baseY * sinA;
            const float rotY = baseX * sinA + baseY * cosA;

            const int px = kCenterX + static_cast<int>(std::lround(rotX));
            const int py = kCenterY + static_cast<int>(std::lround(rotY));

            ctx.gfx.setSafe(px, py, kCrossColor);
            for (int a = 0; a < 4; ++a)
                ctx.gfx.setSafe(px + armDx[a], py + armDy[a], kCrossColor);
        }
    }
}

void BonnehScene::drawMarkers(AppContext &ctx, bool focusVisible) const {
    if (focusVisible)
        ctx.gfx.setSafe(kCenterX, kCenterY, kCenterDotColor);

    ctx.gfx.setSafe(kDistractorBottomX, kDistractorBottomY, kDistractorColor);
    ctx.gfx.setSafe(kDistractorTopLeftX, kDistractorTopLeftY, kDistractorColor);
    ctx.gfx.setSafe(kDistractorTopRightX, kDistractorTopRightY, kDistractorColor);
}

void BonnehScene::setup(AppContext &ctx) {
    rotationDeg_ = 0.0f;
    focusFlashElapsedMs_ = 0.0f;
    drawRotatedLattice(ctx);
    drawMarkers(ctx, /*focusVisible=*/true);
}

void BonnehScene::loop(AppContext &ctx) {
    const float dtMs = ctx.time.dtMs();

    // Advance rotation phase based on elapsed frame time (frame-rate independent)
    rotationDeg_ += 360.0f * kRotationHz * (dtMs / 1000.0f);
    if (rotationDeg_ >= 360.0f)
        rotationDeg_ -= 360.0f;

    // Advance focus-point blink phase
    focusFlashElapsedMs_ += dtMs;
    if (focusFlashElapsedMs_ >= kFocusFlashPeriodMs)
        focusFlashElapsedMs_ -= kFocusFlashPeriodMs;
    const bool focusVisible = focusFlashElapsedMs_ < (kFocusFlashPeriodMs / 2.0f);

    ctx.gfx.clear();

    drawRotatedLattice(ctx);        // background field, drawn first
    drawMarkers(ctx, focusVisible); // markers drawn on top, always visible

    ctx.gfx.show();
}
