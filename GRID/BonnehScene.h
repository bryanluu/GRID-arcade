#ifndef BONNEH_SCENE_H
#define BONNEH_SCENE_H

#include "Colors.h"
#include "Scene.h"

class BonnehScene : public Scene {
  public:
    BonnehScene() = default;
    ~BonnehScene() override = default;
    SceneKind kind() const override { return SceneKind::Bonneh; }
    const char *label() const override { return "Bonneh"; }
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;

  private:
    static constexpr int kCenterX = 15; // 32x32 grid -> even margins
    static constexpr int kCenterY = 15;
    static constexpr int kDistractorBottomX = 15;
    static constexpr int kDistractorBottomY = 25; // ~80% down the grid
    static constexpr int kDistractorTopLeftX = 5;
    static constexpr int kDistractorTopLeftY = 7;
    static constexpr int kDistractorTopRightX = 25;
    static constexpr int kDistractorTopRightY = 7;
    static constexpr Color333 kCenterDotColor = Colors::Bright::Green;
    static constexpr Color333 kDistractorColor = Colors::Bright::Yellow;

    // Rotating background cross field: the whole lattice spins rigidly
    // around (kCenterX, kCenterY), like the reference GIF.
    static constexpr double kRotationHz = 0.1; // matches web version
    static constexpr int kCrossSpacing = 4;    // px between lattice points
    static constexpr int kLatticeRadius = 6;   // lattice extends [-6,6] steps in x & y
    static constexpr int kCrossArmLength = 1;  // px each arm extends
    static constexpr Color333 kCrossColor = Colors::Muted::Blue;

    // Focus-point blink: matches the web version's 500ms steps(2, jump-none)
    // flash, which is a 50% duty-cycle square wave (250ms on, 250ms off).
    static constexpr double kFocusFlashPeriodMs = 500.0;

    double rotationDeg_ = 0.0;         // current phase of the rotating lattice
    double focusFlashElapsedMs_ = 0.0; // current phase within the blink cycle

    void drawRotatedLattice(AppContext &ctx) const;
    void drawCross(AppContext &ctx, int cx, int cy, double angleDeg) const;
    void drawMarkers(AppContext &ctx, bool focusVisible) const;
};

#endif // BONNEH_SCENE_H
