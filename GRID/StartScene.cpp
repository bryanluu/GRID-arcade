#include "StartScene.h"
#include "Colors.h"

// ── Glyph bitmaps (10 px tall), bold blocky to match photo ────────────────────
const uint16_t StartScene::G_cols[kGWidth] = {
    0b0011111111,
    0b0110000000,
    0b1100000000,
    0b1100000000,
    0b1100011111,
    0b1100000011,
    0b0110000011,
    0b0011111100};
const uint16_t StartScene::R_cols[kRWidth] = {
    0b1111111110,
    0b1100000011,
    0b1100000011,
    0b1111111110,
    0b1100110000,
    0b1100011000,
    0b1100001100,
    0b1100000110};
const uint16_t StartScene::I_cols[kIWidth] = {
    0b1111111100,
    0b0001100000,
    0b0001100000,
    0b1111111100};
const uint16_t StartScene::D_cols[kDWidth] = {
    0b1111111100,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1111111100};

const StartScene::Glyph StartScene::kWord[kWordLetters] = {
    {StartScene::G_cols, kGWidth},
    {StartScene::R_cols, kRWidth},
    {StartScene::I_cols, kIWidth},
    {StartScene::D_cols, kDWidth},
};

void StartScene::setup(AppContext &ctx)
{
    // prepare "Welcome to the" scroll text
    const int scale = 1;
    // nudge up so GRID has room below
    const int yTop = ScrollText::yTopCentered(scale) - kBannerShiftUp;
    banner_.prepare(ctx.gfx, "Welcome to ", scale, Colors::Muted::White, Colors::Black, true, false);
    banner_.reset(MATRIX_WIDTH, std::max(0, yTop));

    // prepare GRID title bookkeeping
    totalCols_ = 0;
    for (int i = 0; i < kWordLetters; i++)
        totalCols_ += kWord[i].w;
    totalCols_ += kGapCols * 3;

    revealCols_ = 0;
}

void StartScene::drawGRID(AppContext &ctx, int visibleCols)
{
    // Draw into lower band (rows 12..21) with dim background for nice look
    const Color333 FG = Colors::Bright::Green;

    // Clear band
    ctx.gfx.fillRect(0, kBandTop, MATRIX_WIDTH, kGlyphH, Colors::Black);

    // Final word left edge centered
    const int total = totalCols_;
    const int left = (MATRIX_WIDTH - total) / 2;
    const int xStart = left + std::max(0, total - visibleCols); // reveal from right

    // Iterate glyphs, emit only columns inside [xStart, xStart+visibleCols)
    int cursor = left;
    for (int gi = 0; gi < kWordLetters; ++gi)
    {
        if (gi > 0)
            cursor += kGapCols; // inter-letter gap
        const Glyph &g = kWord[gi];
        for (int c = 0; c < g.w; ++c, ++cursor)
        {
            if (cursor < xStart || cursor >= left + total)
                continue;
            const uint16_t bits = g.cols[c]; // top bit corresponds to row 0
            for (int r = 0; r < kGlyphH; ++r)
            {
                const bool on = (bits >> r) & 1;
                ctx.gfx.setSafe(cursor, kBandTop + r, on ? FG : Colors::Black);
            }
        }
    }
}

void StartScene::loop(AppContext &ctx)
{
    // 1) show scrollText intro
    banner_.step(ctx.gfx, -1); // scroll to left
    bool bannerDone = (banner_.rightEdge() < MATRIX_WIDTH);
    // 2) show GRID glyph
    if (bannerDone)
    {
        revealCols_ = totalCols_;
        drawGRID(ctx, revealCols_);
    }
    // 3) pause
    // 4) transition to MenuScene
}
