#ifndef SCROLL_TEXT_HELPER_H
#define SCROLL_TEXT_HELPER_H

#include "Matrix32.h"

/**
 * @brief Horizontally scrolling, single-line banner renderer.
 *
 * Usage:
 *   ScrollText s;
 *   s.prepare("Hello", 1, Colors::Bright::White, Colors::Black, true, true);
 *   s.reset(MATRIX_WIDTH, ScrollText::yTopCentered(s.ts));
 *   while (running) { s.step(-1, gfx); }
 */
struct ScrollText
{
    /// Cached glyph columns (1-bit per pixel, 5x7 font, with inter-glyph spacing).
    std::vector<PixelMap> cols;

    /// Text scale (integer, >= 1).
    int ts{1};
    /// Foreground (text) color.
    Color333 fg{Colors::Bright::White};
    /// Background band color.
    Color333 bg{Colors::Black};
    /// Whether to fill/clear the text band each frame.
    bool useBg{true};
    /// Looping mode: wrap seamlessly when the banner exits left.
    bool loop{false};

    /// Current left edge of the banner, in pixels.
    int x{MATRIX_WIDTH};
    /// Top Y of the text band, in pixels.
    int y{0};

    /**
     * @brief Height in pixels of the text band for the given scale.
     * @param ts  Text scale.
     * @return    Band height in pixels.
     */
    static inline int bandHeightPx(int ts) { return FONT_GLYPH_HEIGHT * ts; }

    /**
     * @brief Compute a top Y that vertically centers the band on the display.
     * @param ts  Text scale.
     * @return    Top Y for vertical centering.
     */
    static inline int yTopCentered(int ts)
    {
        const int band = bandHeightPx(ts);
        return std::max(0, (MATRIX_HEIGHT - band) / 2);
    }

    /**
     * @brief Build and cache the banner’s columns and set visuals.
     *
     * Newlines are ignored for single-line banners.
     *
     * @param m                Matrix32 instance to use for graphics
     * @param message          C-string text to scroll.
     * @param scale            Integer scale for the 5x7 font.
     * @param color            Foreground text color.
     * @param bgColor          Background band color (default Colors::Black).
     * @param fillBackground   If true, fills the text band each frame.
     * @param shouldLoop       If true, enables seamless looping.
     */
    void prepare(Matrix32 &m, const char *message, int scale, Color333 color,
                 Color333 bgColor = Colors::Black, bool fillBackground = true, bool shouldLoop = false)
    {
        ts = std::max(1, scale);
        fg = color;
        bg = bgColor;
        useBg = fillBackground;
        loop = shouldLoop;
        m.buildStringCols(message, cols);
    }

    /**
     * @brief Initialize scroll position and band placement.
     *
     * @param startX  Initial left edge (e.g., MATRIX_WIDTH to start off-screen right).
     * @param yTop    Vertical top of the band (use yTopCentered(ts) to center).
     */
    void reset(int startX = MATRIX_WIDTH, int yTop = 0)
    {
        x = startX;
        y = yTop;
    }

    /**
     * @brief Render one frame and advance position.
     *
     * Ensures one present per frame when Matrix32::immediate == false.
     * If looping, draws a wrapped copy offset by the full text width.
     *
     * @param m   Matrix32 target renderer.
     * @param dx  Horizontal delta in pixels per call (negative to scroll left).
     * @return    If loop==false: true when the banner has fully exited left.
     *            If loop==true: always false (continuous).
     */
    bool step(Matrix32 &m, int dx)
    {
        if (useBg)
        {
            m.fillRect(0, y, MATRIX_WIDTH, bandHeightPx(ts), bg);
        }

        if (!cols.empty())
        {
            const int totalCols = int(cols.size());
            const int totalPx = totalCols * ts;

            const int firstCol = std::max(0, (-x + (ts - 1)) / ts);
            const int lastCol = std::min(totalCols, (MATRIX_WIDTH - x + ts - 1) / ts);

            if (firstCol < lastCol)
            {
                const int x0 = x + firstCol * ts;
                m.blitCols(x0, y, cols.data() + firstCol, lastCol - firstCol, fg, ts);
            }

            if (loop)
            {
                const int xWrap = x + totalPx;
                if (xWrap < MATRIX_WIDTH)
                {
                    const int f2 = std::max(0, (-xWrap + (ts - 1)) / ts);
                    const int l2 = std::min(totalCols, (MATRIX_WIDTH - xWrap + ts - 1) / ts);
                    if (f2 < l2)
                    {
                        const int x0b = xWrap + f2 * ts;
                        m.blitCols(x0b, y, cols.data() + f2, l2 - f2, fg, ts);
                    }
                }
            }
        }

        m.show();
        x += dx;

        const int rightEdge = x + int(cols.size()) * ts;
        if (loop)
        {
            if (rightEdge <= 0)
                x += int(cols.size()) * ts;
            return false;
        }
        return (rightEdge < 0);
    }
};

#endif // SCROLL_TEXT_HELPER_H
