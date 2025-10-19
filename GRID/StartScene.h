#ifndef GRID_STARTSCENE_H
#define GRID_STARTSCENE_H

#include "Scene.h"
#include "ScrollTextHelper.h"

class StartScene final : public Scene
{
public:
    StartScene() = default;

    SceneTimingPrefs timingPrefs() const override
    {
        // Default 60Hz is fine; return NaN to keep global default
        return SceneTimingPrefs(std::numeric_limits<double>::quiet_NaN());
    }

    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;

private:
    // Dimensions and timings for StartScene
    static const uint8_t kGlyphH = 10;     // pixel height of custom glyphs
    static const uint8_t kGapCols = 2;     // spacing between letters (columns)
    static const uint8_t kWordLetters = 4; // letters in "GRID"
    static const uint32_t kHoldMs = 3000;  // hold duration after fully visible (ms)
    static const int kBandTop = 12;        // Y top for GRID band
    static const int kBannerShiftUp = 6;   // shift "Welcome to the" upward

    // Per-letter widths (columns)
    static const uint8_t kGWidth = 8;
    static const uint8_t kRWidth = 8;
    static const uint8_t kIWidth = 4;
    static const uint8_t kDWidth = 8;

    // Column tables use the matching widths above
    static const uint16_t G_cols[kGWidth];
    static const uint16_t R_cols[kRWidth];
    static const uint16_t I_cols[kIWidth];
    static const uint16_t D_cols[kDWidth];

    struct Glyph
    {
        const uint16_t *cols;
        uint8_t w;
    };
    static const Glyph kWord[kWordLetters]; // = { G, R, I, D }

    ScrollText banner_;

    uint8_t totalCols_{0};  // total columns (word + gaps)
    uint8_t revealCols_{0}; // columns currently visible from the right

    void drawGRID(AppContext &ctx, int visibleCols);
};

#endif // GRID_STARTSCENE_H
