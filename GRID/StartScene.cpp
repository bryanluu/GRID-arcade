#include "StartScene.h"
#include "Colors.h"

// ── Glyph bitmaps (10 px tall), bold blocky to match photo ────────────────────
const uint16_t StartScene::G_cols[kGWidth] = {
    0b0011111100,
    0b0011111100,
    0b1100000011,
    0b1100000011,
    0b1100110011,
    0b1100110011,
    0b0011110011,
    0b0011110011};
const uint16_t StartScene::R_cols[kRWidth] = {
    0b1111111111,
    0b1111111111,
    0b0000110011,
    0b0000110011,
    0b0011110011,
    0b0011110011,
    0b1100001100,
    0b1100001100};
const uint16_t StartScene::I_cols[kIWidth] = {
    0b1111111111,
    0b1111111111};
const uint16_t StartScene::D_cols[kDWidth] = {
    0b1111111111,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b0011111100,
    0b0011111100};

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
    totalCols_ += kGapCols * (kWordLetters - 1);

    revealCols_ = 0;
}

// Batch-draw contiguous "on" runs per column (column-major; bit 0 = top).
static inline void blitCols(Matrix32 &gfx,
                            int x0, int y0,
                            const uint16_t *cols, int nCols,
                            Color333 fg,
                            int bandHeight /*= kGlyphH*/)
{
    for (int ci = 0; ci < nCols; ++ci)
    {
        const int x = x0 + ci; // ts = 1 (no horizontal scale)
        if (x >= MATRIX_WIDTH)
            break; // fully off-right
        if (x < 0)
            continue; // fully off-left

        uint16_t bits = cols[ci];
        int row = 0;
        while (row < bandHeight)
        {
            // find next lit run
            while (row < bandHeight && ((bits & (1u << row)) == 0))
                ++row;
            if (row >= bandHeight)
                break;
            const int runStart = row;
            while (row < bandHeight && (bits & (1u << row)))
                ++row;
            const int runLen = row - runStart;

            // draw the run as one vertical rectangle (ts = 1)
            const int y = y0 + runStart;
            // Safe fill: clip top/bottom once
            int y1 = std::max(0, y);
            int y2 = std::min(MATRIX_HEIGHT, y + runLen);
            if (y2 > y1)
                gfx.fillRect(x, y1, 1, y2 - y1, fg);
        }
    }
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

        // Compute the slice of this glyph that is currently visible
        const int glyphX0 = cursor;
        const int colFirst = std::max(0, xStart - glyphX0);
        const int colLastExc = std::min<int>(g.w, xStart + visibleCols - glyphX0);
        const int sliceCols = colLastExc - colFirst;

        if (sliceCols > 0)
        {
            // Batch draw: vertical span rectangles instead of per-pixel plotting
            blitCols(ctx.gfx,
                     glyphX0 + colFirst, // x0 on screen
                     kBandTop,           // y0 (band top)
                     g.cols + colFirst,  // starting column pointer
                     sliceCols,          // number of columns to draw
                     FG,
                     kGlyphH);
        }

        cursor += g.w;
    }
}

void StartScene::loop(AppContext &ctx)
{
    // 1) show scrollText intro
    if (!bannerDone_)
        bannerDone_ = banner_.step(ctx.gfx, -1); // scroll to left
    // 2) show GRID glyph
    if (bannerDone_)
    {
        revealCols_ = totalCols_;
        drawGRID(ctx, revealCols_);
    }
    // 3) pause
    // 4) transition to MenuScene
}
