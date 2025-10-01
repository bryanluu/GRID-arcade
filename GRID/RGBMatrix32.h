#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include "Matrix32.h"
#include <RGBmatrixPanel.h>

using PanelColor = uint16_t;

// Adapter that wraps an existing Adafruit RGBmatrixPanel
class RGBMatrix32 : public Matrix32
{
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
                m.drawPixel(x, y, fb_[y * MATRIX_WIDTH + x]);
    }

    // Drawing API (1:1 to Adafruit panel)

    void drawPixel(int x, int y, Color333 c) override
    {
        set(x, y, c);
        if (immediate) show();
    }
    void drawLine(int x0, int y0, int x1, int y1, Color333 c) override
    {
        m.drawLine(x0, y0, x1, y1, convertColor(c));
    }
    void drawRect(int x, int y, int w, int h, Color333 c) override
    {
        m.drawRect(x, y, w, h, convertColor(c));
    }
    void drawCircle(int cx, int cy, int r, Color333 c) override
    {
        m.drawCircle(cx, cy, r, convertColor(c));
    }
    void fillRect(int x, int y, int w, int h, Color333 c) override
    {
        m.fillRect(x, y, w, h, convertColor(c));
    }
    void fillCircle(int cx, int cy, int r, Color333 c) override
    {
        m.fillCircle(cx, cy, r, convertColor(c));
    }

    // Text helpers

    void setCursor(int x, int y) override { m.setCursor(x, y); }
    void setTextColor(Color333 c) override { m.setTextColor(convertColor(c)); }
    void setTextSize(int s) override { m.setTextSize(s); }
    void print(char ch) override { m.print(ch); }
    void print(const char *s) override { m.print(s); }
    void println(const char *s) override { m.println(s); }

    void advance() override { m.setCursor(m.getCursorX() + 5, m.getCursorY()); }
    void drawChar(int x, int y, char ch, Color333 c) override
    {
        int savedX = x, savedY = y;
        setCursor(x, y);
        setTextColor(c);
        print(ch);
        setCursor(savedX, savedY);
    }

    // Converts a Color333 to the type used by the panel
    PanelColor convertColor(Color333 c)
    {
        return m.Color333(c.r, c.g, c.b);
    }

private:
    RGBmatrixPanel &m; // reference to a live panel
};

#endif // RGB_MATRIX_H
