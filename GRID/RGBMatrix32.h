#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include "Matrix32.h"
#include <RGBmatrixPanel.h>

using PanelColor = uint16_t;

// 5x7 ASCII font declaration (defined elsewhere)
extern const PixelMap FONT5x7[96][5];

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

    void span(int x0, int x1, int y, Color333 c);
    void drawHLine(int x, int y, int w, Color333 c);
    void drawVLine(int x, int y, int h, Color333 c);
    void plot8(int cx, int cy, int x, int y, Color333 c);
    void drawPixelScaled(int x, int y, Color333 c);

public:
    // Panel must outlive this adapter
    explicit RGBMatrix32(RGBmatrixPanel &panel) : m(panel) {}

    // 32x32 RGB framebuffer (row-major)
    PanelColor fb_[MATRIX_WIDTH * MATRIX_HEIGHT]{};
    // Convert (x,y) to framebuffer index.
    static constexpr int coordToIndex(int x, int y)
    {
        return y * MATRIX_WIDTH + x;
    }
    PanelColor get(int x, int y) const { return fb_[coordToIndex(x, y)]; }
    // Converts a Color333 to the type used by the panel
    PanelColor convertColor(Color333 c)
    {
        return m.Color333(c.r, c.g, c.b);
    }

    // Matrix32 interface

    void begin() override;
    void clear() override;
    void set(int x, int y, Color333 c) override;
    void show() override;
    
    // Drawing API (1:1 to Adafruit panel)

    void drawPixel(int x, int y, Color333 c) override;
    void drawLine(int x0, int y0, int x1, int y1, Color333 c) override;
    void drawRect(int x, int y, int w, int h, Color333 c) override;
    void drawCircle(int cx, int cy, int r, Color333 c) override;
    void fillRect(int x, int y, int w, int h, Color333 c) override;
    void fillCircle(int cx, int cy, int r, Color333 c) override;

    // Text helpers

    void advance() override;
    void setCursor(int x, int y);
    void setTextColor(Color333 c) override;
    void setTextSize(int s) override;
    void drawChar(int x, int y, char ch, Color333 c) override;
    void print(char ch) override;
    void print(const char *s) override;
    void println(const char *s) override;
};

#endif // RGB_MATRIX_H
