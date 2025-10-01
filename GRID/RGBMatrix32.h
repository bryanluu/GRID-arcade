#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include "Matrix32.h"
#include <algorithm>
#include <RGBmatrixPanel.h>

using PanelColor = uint16_t;

// Adapter that wraps an existing Adafruit RGBmatrixPanel
class RGBMatrix32 : public Matrix32
{

    RGBmatrixPanel &m; // reference to a live panel

    // Text state
    int cx{0};                      // cursor x in pixels
    int cy{0};                      // cursor y in pixels
    int lineStartX{0};              // start-of-line x for newline handling
    int ts{1};                      // text scale
    Color333 tc{Color333{7, 7, 7}}; // text color

    // Low-level framebuffer helpers

    // Draw a clamped horizontal span of pixels in the framebuffer
    void span(int x0, int x1, int y, Color333 c)
    {
        if (y < 0 || y >= MATRIX_HEIGHT)
            return;
        x0 = std::max(0, x0);
        x1 = std::min(MATRIX_WIDTH - 1, x1);
        for (int x = x0; x <= x1; ++x)
            setSafe(x, y, c);
    }

    // Draw a horizontal line in the framebuffer
    void drawHLine(int x, int y, int w, Color333 c)
    {
        for (int i = 0; i < w; ++i)
            setSafe(x + i, y, c);
    }

    // Draw a vertical line in the framebuffer
    void drawVLine(int x, int y, int h, Color333 c)
    {
        for (int i = 0; i < h; ++i)
            setSafe(x, y + i, c);
    }
    // Plot using 8-way symmetry for circle algorithms
    void plot8(int cx, int cy, int x, int y, Color333 c)
    {
        setSafe(cx + x, cy + y, c);
        setSafe(cx - x, cy + y, c);
        setSafe(cx + x, cy - y, c);
        setSafe(cx - x, cy - y, c);
        setSafe(cx + y, cy + x, c);
        setSafe(cx - y, cy + x, c);
        setSafe(cx + y, cy - x, c);
        setSafe(cx - y, cy - x, c);
    }
    // draw ts x ts block
    void drawPixelScaled(int x, int y, Color333 c)
    {
        for (int dy = 0; dy < ts; ++dy)
            for (int dx = 0; dx < ts; ++dx)
                setSafe(x * ts + dx, y * ts + dy, c);
    }

public:
    // Panel must outlive this adapter
    explicit RGBMatrix32(RGBmatrixPanel &panel) : m(panel) {}

    // 32x32 RGB framebuffer (row-major)
    PanelColor fb_[MATRIX_WIDTH * MATRIX_HEIGHT]{};

    // Matrix32 interface

    void clear() override
    {
        for (auto &p : fb_)
            p = 0;
    }
    void begin() override
    {
        m.begin();
        clear();
    }
    // Convert (x,y) to framebuffer index.
    static constexpr int coordToIndex(int x, int y) { return y * MATRIX_WIDTH + x; }
    PanelColor get(int x, int y) const { return fb_[coordToIndex(x, y)]; }
    void set(int x, int y, Color333 c) override
    {
        fb_[coordToIndex(x, y)] = convertColor(c);
    }
    void show() override
    {
        // Copy framebuffer to panel
        for (byte y = 0; y < MATRIX_HEIGHT; ++y)
            for (byte x = 0; x < MATRIX_WIDTH; ++x)
                m.drawPixel(x, y, fb_[coordToIndex(x, y)]);
    }

    // Drawing API (1:1 to Adafruit panel)

    // Alias for set()
    void drawPixel(int x, int y, Color333 c) override
    {
        setSafe(x, y, c);
        if (immediate)
            show();
    }

    // Bresenham line
    void drawLine(int x0, int y0, int x1, int y1, Color333 c) override
    {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true)
        {
            setSafe(x0, y0, c);
            if (x0 == x1 && y0 == y1)
                break;
            int e2 = 2 * err;
            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
        if (immediate)
            show();
    }

    // Rectangle outline
    void drawRect(int x, int y, int w, int h, Color333 c) override
    {
        if (w <= 0 || h <= 0)
            return;
        drawHLine(x, y, w, c);
        drawHLine(x, y + h - 1, w, c);
        drawVLine(x, y, h, c);
        drawVLine(x + w - 1, y, h, c);
        if (immediate)
            show();
    }

    // Midpoint circle outline
    void drawCircle(int cx, int cy, int r, Color333 c) override
    {
        if (r < 0)
            return;
        int x = r, y = 0, err = 1 - r;
        while (x >= y)
        {
            plot8(cx, cy, x, y, c);
            ++y;
            if (err < 0)
                err += 2 * y + 1;
            else
            {
                --x;
                err += 2 * (y - x) + 1;
            }
        }
        if (immediate)
            show();
    }

    // Filled rectangle
    void fillRect(int x, int y, int w, int h, Color333 c) override
    {
        int x0 = std::max(0, x), y0 = std::max(0, y);
        int x1 = std::min(MATRIX_WIDTH - 1, x + w - 1), y1 = std::min(MATRIX_HEIGHT - 1, y + h - 1);
        for (int yy = y0; yy <= y1; ++yy)
            for (int xx = x0; xx <= x1; ++xx)
                setSafe(xx, yy, c);
        if (immediate)
            show();
    }

    // Filled circle via spans
    void fillCircle(int cx, int cy, int r, Color333 c) override
    {
        if (r < 0)
            return;
        int x = r, y = 0, err = 1 - r;
        while (x >= y)
        {
            span(cx - x, cx + x, cy + y, c);
            span(cx - x, cx + x, cy - y, c);
            span(cx - y, cx + y, cy + x, c);
            span(cx - y, cx + y, cy - x, c);
            ++y;
            if (err < 0)
                err += 2 * y + 1;
            else
            {
                --x;
                err += 2 * (y - x) + 1;
            }
        }
        if (immediate)
            show();
    }

    // Text helpers

    // Move cursor by 1 glyph (5px + 1px spacing) at current scale
    void advance() override { cx += ts * 6; }

    // Set text cursor
    void setCursor(int x, int y) override
    {
        cx = x;
        cy = y;
        lineStartX = x;
    }

    // Set text color
    void setTextColor(Color333 c) override { tc = c; }

    // Set integer text scale >= 1
    void setTextSize(int s) override { ts = std::max(1, s); }

    // Draw a 5x7 glyph scaled by setTextSize() at (x,y)
    void drawChar(int x, int y, char ch, Color333 c) override
    {
        int savedX = x, savedY = y;
        setCursor(x, y);
        setTextColor(c);
        print(ch);
        setCursor(savedX, savedY);
    }

    // Print a single character (handles newline)
    void print(char ch) override
    {
        if (ch == '\n')
        {
            cx = lineStartX;
            cy += ts * 8;
            return;
        }
        if (ch < ASCII_START)
        {
            advance();
            return;
        }
        drawChar(cx, cy, ch, tc);
        advance();
    }

    // Print a C string
    void print(const char *s) override
    {
        for (const char *p = s; *p; ++p)
            print(*p);
    }

    // Print a C string then newline
    void println(const char *s) override
    {
        print(s);
        print('\n');
    }

    // Converts a Color333 to the type used by the panel
    PanelColor convertColor(Color333 c)
    {
        return m.Color333(c.r, c.g, c.b);
    }
};

#endif // RGB_MATRIX_H
