#ifndef MATRIX32_H
#define MATRIX32_H

#include "Colors.h"
#include <cstdint>
#include <vector>

#define MATRIX_SIZE 32
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32
#define ASCII_START 32
#define FONT_GLYPH_WIDTH 5
#define FONT_GLYPH_HEIGHT 7
#define FONT_CHAR_WIDTH 6
#define FONT_CHAR_HEIGHT 7

using MatrixPosition = uint8_t;
using PixelMap = uint8_t;
using PixelColumn = uint8_t;

// 5x7 ASCII font declaration (defined elsewhere)
extern const PixelMap FONT5x7[96][5];

class Matrix32
{
public:
    virtual void begin() = 0;
    virtual void clear() = 0;

    // Set a pixel (bounds are NOT checked)
    virtual void set(int x, int y, Color333 c) = 0;

    // Safe setter with bounds check
    inline void setSafe(int x, int y, Color333 c)
    {
        if (0 <= x && x < MATRIX_WIDTH && 0 <= y && y < MATRIX_HEIGHT)
            set(x, y, c);
    }

    // Immediate mode control: if true, show() is called after each draw operation.
    virtual void setImmediate(bool on) { immediate = on; }
    virtual void show() = 0;

    // Drawing API
    virtual void drawChar(int x, int y, char ch, Color333 c) = 0;
    virtual void drawPixel(int x, int y, Color333 c) = 0;
    virtual void drawLine(int x0, int y0, int x1, int y1, Color333 c) = 0;
    virtual void drawRect(int x, int y, int w, int h, Color333 c) = 0;
    virtual void drawCircle(int cx, int cy, int r, Color333 c) = 0;
    virtual void fillRect(int x, int y, int w, int h, Color333 c) = 0;
    virtual void fillCircle(int cx, int cy, int r, Color333 c) = 0;

    // Text helpers
    virtual void advance() = 0;
    virtual void setCursor(int x, int y) = 0;
    virtual void setTextColor(Color333 c) = 0;
    virtual void setTextSize(int s) = 0;
    virtual void print(char ch) = 0;
    virtual void print(const char *s) = 0;
    virtual void println(const char *s) = 0;

    /**
     * Build the 5 column bitmasks for a single 5x7 glyph.
     *
     * Each byte in outCols uses the low 7 bits to represent a column,
     * where bit 0 is the top row and bit 6 is the bottom row.
     *
     * @param ch        ASCII character (expects ch >= ASCII_START).
     * @param outCols   Output array of 5 column masks.
     */
    inline void buildGlyphCols(char ch, PixelMap outCols[FONT_GLYPH_WIDTH])
    {
        // Non-printables become empty columns
        if (ch < ASCII_START)
        {
            for (int i = 0; i < FONT_GLYPH_WIDTH; ++i)
                outCols[i] = 0;
            return;
        }
        const PixelMap *g = FONT5x7[ch - ASCII_START];
        for (int i = 0; i < FONT_GLYPH_WIDTH; ++i)
            outCols[i] = g[i];
    }

    /**
     * Convert a C-string into a flat array of column bitmasks with 1 empty column as spacing.
     *
     * Newlines are ignored for single-line banners. The vector ends without a trailing space.
     *
     * @param s     Input text.
     * @param cols  Output vector of column masks (5 per glyph plus 1 spacer between glyphs).
     */
    inline void buildStringCols(const char *s, std::vector<PixelMap> &cols)
    {
        cols.clear();
        for (const char *p = s; *p; ++p)
        {
            if (*p == '\n')
                continue; // single line use-case
            PixelMap g[FONT_GLYPH_WIDTH];
            buildGlyphCols(*p, g);
            for (int i = 0; i < FONT_GLYPH_WIDTH; ++i)
                cols.push_back(g[i]);
            cols.push_back(0); // 1-column spacing between glyphs
        }
        // Remove trailing space if present
        if (!cols.empty())
            cols.pop_back();
    }

    /**
     * Draw precomputed text columns using vertical span batching.
     *
     * Strategy: for each column, find contiguous runs of set bits and render each run
     * as one filled rectangle. This minimizes per-pixel calls.
     *
     * @param x0     Top-left X of the text band in pixels.
     * @param y0     Top-left Y of the text band in pixels.
     * @param cols   Pointer to column masks.
     * @param nCols  Number of columns to draw from cols.
     * @param c      Foreground color.
     * @param ts     Integer scale factor (>= 1).
     */
    inline void blitCols(int x0, int y0, const PixelMap *cols, int nCols, Color333 c, int ts)
    {
        for (int ci = 0; ci < nCols; ++ci)
        {
            const int x = x0 + ci * ts;
            // Skip columns fully off to the right; break when first fully off-right
            if (x >= MATRIX_WIDTH)
                break;
            // Skip columns fully off to the left
            if (x + ts <= 0)
                continue;

            PixelMap bits = cols[ci];
            int row = 0;
            while (row < FONT_GLYPH_HEIGHT)
            {
                // Find start of next lit run
                while (row < FONT_GLYPH_HEIGHT && !(bits & (1u << row)))
                    ++row;
                if (row >= FONT_GLYPH_HEIGHT)
                    break;

                const int runStart = row;
                // Extend to end of run
                while (row < FONT_GLYPH_HEIGHT && (bits & (1u << row)))
                    ++row;
                const int runLen = row - runStart;

                // Draw the scaled vertical run as one rectangle
                const int y = y0 + runStart * ts;
                fillRect(x, y, ts, runLen * ts, c);
            }
        }
    }

    virtual ~Matrix32() = default;

protected:
    bool immediate{false}; // if true, show() after each draw operation
};

#endif // MATRIX32_H
