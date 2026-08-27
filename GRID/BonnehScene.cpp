#include "BonnehScene.h"
#include "Matrix32.h"
#include <cmath>

void BonnehScene::drawCross(AppContext &ctx, int cx, int cy, double angleDeg) const {
    ctx.gfx.setSafe(cx, cy, kCrossColor);
    for (int i = 0; i < 4; ++i) {
        const double rad = (angleDeg + i * 90.0) * M_PI / 180.0;
        const int dx = static_cast<int>(std::lround(std::cos(rad) * kCrossArmLength));
        const int dy = static_cast<int>(std::lround(std::sin(rad) * kCrossArmLength));
        ctx.gfx.setSafe(cx + dx, cy + dy, kCrossColor);
    }
}

void BonnehScene::drawRotatedLattice(AppContext &ctx) const {
    const double rad = rotationDeg_ * M_PI / 180.0;
    const double cosA = std::cos(rad);
    const double sinA = std::sin(rad);

    for (int j = -kLatticeRadius; j <= kLatticeRadius; ++j) {
        for (int i = -kLatticeRadius; i <= kLatticeRadius; ++i) {
            // Base lattice offset from center (unrotated)
            const double baseX = i * kCrossSpacing;
            const double baseY = j * kCrossSpacing;

            // Rotate the offset rigidly around the center
            const double rotX = baseX * cosA - baseY * sinA;
            const double rotY = baseX * sinA + baseY * cosA;

            const int px = kCenterX + static_cast<int>(std::lround(rotX));
            const int py = kCenterY + static_cast<int>(std::lround(rotY));

            // Each cross's own arms rotate with the lattice too
            drawCross(ctx, px, py, rotationDeg_);
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
    rotationDeg_ = 0.0;
    focusFlashElapsedMs_ = 0.0;
    drawRotatedLattice(ctx);
    drawMarkers(ctx, /*focusVisible=*/true);
}

void BonnehScene::loop(AppContext &ctx) {
    const double dtMs = ctx.time.dtMs();

    // Advance rotation phase based on elapsed frame time (frame-rate independent)
    rotationDeg_ += 360.0 * kRotationHz * (dtMs / 1000.0);
    if (rotationDeg_ >= 360.0)
        rotationDeg_ -= 360.0;

    // Advance focus-point blink phase
    focusFlashElapsedMs_ += dtMs;
    if (focusFlashElapsedMs_ >= kFocusFlashPeriodMs)
        focusFlashElapsedMs_ -= kFocusFlashPeriodMs;
    const bool focusVisible = focusFlashElapsedMs_ < (kFocusFlashPeriodMs / 2.0);

    ctx.gfx.clear();

    drawRotatedLattice(ctx);        // background field, drawn first
    drawMarkers(ctx, focusVisible); // markers drawn on top, always visible

    ctx.gfx.show();
}
