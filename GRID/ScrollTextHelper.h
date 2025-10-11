#ifndef SCROLL_TEXT_HELPER_H
#define SCROLL_TEXT_HELPER_H

#include "Matrix32.h"

struct ScrollText
{
    std::vector<PixelMap> cols;
    int ts{1};
    Color333 fg{WHITE};
    Color333 bg{BLACK};
    bool useBg{true};
    bool loop{false};

    int x{MATRIX_WIDTH}; // head position (left edge of text)
    int y{0};            // top y of band (scaled)

    // --- Utilities ---
    static inline int bandHeightPx(int ts) { return FONT_GLYPH_HEIGHT * ts; }
    static inline int yTopCentered(int ts)
    {
        const int band = bandHeightPx(ts);
        return std::max(0, (MATRIX_HEIGHT - band) / 2);
    }

    // --- API ---
    void prepare(Matrix32 &m, const char *message, int scale, Color333 color, Color333 bgColor = BLACK, bool fillBackground = true, bool shouldLoop = false)
    {
        ts = std::max(1, scale);
        fg = color;
        bg = bgColor;
        useBg = fillBackground;
        loop = shouldLoop;
        m.buildStringCols(message, cols);
    }

    void reset(int startX = MATRIX_WIDTH, int yTop = 0)
    {
        x = startX;
        y = yTop;
    }

    // dx negative to scroll left. Returns true when finished (non-loop), or always false when looping.
    bool step(Matrix32 &m, int dx)
    {
        // Optional background band clear/fill
        if (useBg)
        {
            m.fillRect(0, y, MATRIX_WIDTH, bandHeightPx(ts), bg);
        }

        if (!cols.empty())
        {
            const int totalCols = int(cols.size());
            const int totalPx = totalCols * ts;

            // Visible slice in columns
            int firstCol = std::max(0, (-x + (ts - 1)) / ts);
            int lastCol = std::min(totalCols, (MATRIX_WIDTH - x + ts - 1) / ts);
            if (firstCol < lastCol)
            {
                const int x0 = x + firstCol * ts;
                m.blitCols(x0, y, cols.data() + firstCol, lastCol - firstCol, fg, ts);
            }

            // If looping and the banner has partially or fully left the screen,
            // draw a wrapped copy offset by totalPx to keep continuous scroll.
            if (loop)
            {
                int x2 = x + totalPx; // next copy to the right
                if (x2 < MATRIX_WIDTH)
                {
                    int f2 = std::max(0, (-x2 + (ts - 1)) / ts);
                    int l2 = std::min(totalCols, (MATRIX_WIDTH - x2 + ts - 1) / ts);
                    if (f2 < l2)
                    {
                        const int x0b = x2 + f2 * ts;
                        m.blitCols(x0b, y, cols.data() + f2, l2 - f2, fg, ts);
                    }
                }
            }
        }

        m.show();
        x += dx;

        // Termination
        const int rightEdge = x + int(cols.size()) * ts;
        if (loop)
        {
            // When fully off left, wrap x back by the text width to continue seamlessly
            if (rightEdge <= 0)
                x += int(cols.size()) * ts;
            return false; // looping never "finishes"
        }
        return (rightEdge < 0);
    }
};

#endif // SCROLL_TEXT_HELPER_H
