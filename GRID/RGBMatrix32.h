#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include "Matrix32.h"
#include <RGBmatrixPanel.h>

// Adapter that wraps an existing Adafruit RGBmatrixPanel
class RGBMatrix32 : public Matrix32 {
public:
    // Panel must outlive this adapter
    explicit RGBMatrix32(RGBmatrixPanel& panel) : m(panel) {}

    // Matrix32 interface
    void begin() override { m.begin(); }
    void clear() override { m.fillScreen(0); }
    void set(int x, int y, RGB c) override { m.drawPixel(x, y, color(c)); }
    void show() override { /* hardware panel draws immediately; no-op */ }

    // Drawing API (1:1 to Adafruit panel)
    void drawPixel(int x, int y, RGB c) override { m.drawPixel(x, y, color(c)); }
    void drawLine(int x0, int y0, int x1, int y1, RGB c) override { m.drawLine(x0, y0, x1, y1, color(c)); }
    void drawRect(int x, int y, int w, int h, RGB c) override { m.drawRect(x, y, w, h, color(c)); }
    void drawCircle(int cx, int cy, int r, RGB c) override { m.drawCircle(cx, cy, r, color(c)); }
    void fillRect(int x, int y, int w, int h, RGB c) override { m.fillRect(x, y, w, h, color(c)); }
    void fillCircle(int cx, int cy, int r, RGB c) override { m.fillCircle(cx, cy, r, color(c)); }

    // Text helpers
    void setCursor(int x, int y) override { m.setCursor(x, y); }
    void setTextColor(RGB c) override { m.setTextColor(color(c)); }
    void setTextSize(int s) override { m.setTextSize(s); }
    void print(char ch) override { m.print(ch); }
    void print(const char* s) override { m.print(s); }
    void println(const char* s) override { m.println(s); }

    // If your scenes rely on these, provide trivial impls or route to panel text
    void advance() override { /* scenes can manage their own cursor advance */ }
    void drawChar(int x, int y, char ch, RGB c) override {
        int savedX = x, savedY = y;
        m.setCursor(x, y);
        m.setTextColor(color(c));
        m.print(ch);
        m.setCursor(savedX, savedY);
    }

private:
    // Convert 8-bit RGB to the panel’s 3-3-3 format.
    // Many library versions declare Color333 static; call via class to avoid const issues.
    inline uint16_t color(RGB c) const {
        auto to3 = [](uint8_t v) { return static_cast<uint8_t>((v * 7 + 127) / 255); };
        return m.Color333(to3(c.r), to3(c.g), to3(c.b));
    }

    RGBmatrixPanel& m; // reference to a live panel
};

#endif // RGB_MATRIX_H
