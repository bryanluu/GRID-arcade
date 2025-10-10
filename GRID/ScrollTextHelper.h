#ifndef SCROLL_TEXT_HELPER_H
#define SCROLL_TEXT_HELPER_H

#include "Matrix32.h"

struct ScrollText
{
    std::vector<PixelMap> cols;
    int ts{1};
    Color333 color{WHITE};
    int x{MATRIX_WIDTH}; // head position (left edge of text)
    int y{0};

    void prepare(Matrix32 &m, const char *message, int scale, Color333 c)
    {
        ts = std::max(1, scale);
        color = c;
        m.buildStringCols(message, cols);
    }

    void reset(int startX = MATRIX_WIDTH, int yTop = 0)
    {
        x = startX;
        y = yTop;
    }

    // dx is negative to move left, e.g. dx = -1
    // Returns false while visible or entering, true when fully off-screen to the left
    bool step(Matrix32 &m, int dx)
    {
        // Draw into a minimal band: clear only the text band
        m.fillRect(0, y, MATRIX_WIDTH, FONT_GLYPH_HEIGHT * ts, BLACK);

        if (!cols.empty())
        {
            // Compute visible slice
            const int totalCols = int(cols.size());
            int firstCol = std::max(0, (-x + (ts - 1)) / ts);
            int lastCol = std::min(totalCols, (MATRIX_WIDTH - x + ts - 1) / ts);
            if (firstCol < lastCol)
            {
                int x0 = x + firstCol * ts;
                m.blitCols(x0, y, cols.data() + firstCol, lastCol - firstCol, color, ts);
            }
        }
        m.show();

        x += dx; // advance
        const int rightEdge = x + int(cols.size()) * ts;
        return (rightEdge < 0); // done when fully off-screen
    }
};

#endif // SCROLL_TEXT_HELPER_H
