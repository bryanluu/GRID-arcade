#include "RGBMatrix32.h"
#include <algorithm>

// Draw a clamped horizontal span of pixels in the framebuffer
void RGBMatrix32::span(int x0, int x1, int y, Color333 c)
{
    if (y < 0 || y >= MATRIX_HEIGHT)
        return;
    x0 = std::max(0, x0);
    x1 = std::min(MATRIX_WIDTH - 1, x1);
    for (int x = x0; x <= x1; ++x)
        setSafe(x, y, c);
}

// Draw a horizontal line in the framebuffer
void RGBMatrix32::drawHLine(int x, int y, int w, Color333 c)
{
    for (int i = 0; i < w; ++i)
        setSafe(x + i, y, c);
}

// Draw a vertical line in the framebuffer
void RGBMatrix32::drawVLine(int x, int y, int h, Color333 c)
{
    for (int i = 0; i < h; ++i)
        setSafe(x, y + i, c);
}
// Plot using 8-way symmetry for circle algorithms
void RGBMatrix32::plot8(int cx, int cy, int x, int y, Color333 c)
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
void RGBMatrix32::drawPixelScaled(int x, int y, Color333 c)
{
    for (int dy = 0; dy < ts; ++dy)
        for (int dx = 0; dx < ts; ++dx)
            setSafe(x * ts + dx, y * ts + dy, c);
}

// Matrix32 interface

void RGBMatrix32::begin()
{
    m.begin();
}
void RGBMatrix32::clear()
{
    for (auto &p : fb_)
        p = 0;
}
void RGBMatrix32::set(int x, int y, Color333 c)
{
    fb_[coordToIndex(x, y)] = convertColor(c); // test write to fb_
}
void RGBMatrix32::show()
{
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
        for (int x = 0; x < MATRIX_WIDTH; ++x)
            m.drawPixel(x, y, get(x, y));
}

// Drawing API (1:1 to Adafruit panel)

void RGBMatrix32::drawPixel(int x, int y, Color333 c)
{
    setSafe(x, y, c);
    if (immediate)
        show();
}
void RGBMatrix32::drawLine(int x0, int y0, int x1, int y1, Color333 c)
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
void RGBMatrix32::drawRect(int x, int y, int w, int h, Color333 c)
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
void RGBMatrix32::drawCircle(int cx, int cy, int r, Color333 c)
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
void RGBMatrix32::fillRect(int x, int y, int w, int h, Color333 c)
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
void RGBMatrix32::fillCircle(int cx, int cy, int r, Color333 c)
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
void RGBMatrix32::advance() { cx += ts * 6; }

// Set text cursor
void RGBMatrix32::setCursor(int x, int y)
{
    cx = x;
    cy = y;
    lineStartX = x;
}

// Set text color
void RGBMatrix32::setTextColor(Color333 c) { tc = c; }

// Set integer text scale >= 1
void RGBMatrix32::setTextSize(int s) { ts = std::max(1, s); }

// Draw a 5x7 glyph scaled by setTextSize() at (x,y)
void RGBMatrix32::drawChar(int x, int y, char ch, Color333 c)
{
    const PixelMap *glyph = FONT5x7[ch - ASCII_START];
    for (int col = 0; col < 5; ++col)
    {
        PixelColumn bits = glyph[col];
        for (int row = 0; row < 7; ++row)
            if (bits & (1u << row))
                drawPixelScaled(x + col, y + row, c);
    }
    if (immediate)
        show();
}

// Print a single character (handles newline)
void RGBMatrix32::print(char ch)
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
void RGBMatrix32::print(const char *s)
{
    for (const char *p = s; *p; ++p)
        print(*p);
}

// Print a C string then newline
void RGBMatrix32::println(const char *s)
{
    print(s);
    print('\n');
}
