#ifndef MATRIX32_H
#define MATRIX32_H

#include <cstdint>

struct RGB
{
    uint8_t r, g, b;
};
inline constexpr RGB rgb(uint8_t r, uint8_t g, uint8_t b) { return RGB{r, g, b}; }

class Matrix32
{
public:
    virtual void begin() = 0;
    virtual void clear() = 0;

    // Set a pixel (bounds are NOT checked)
    virtual void set(int x, int y, RGB c) = 0;

    // Safe setter with bounds check
    inline void setSafe(int x, int y, RGB c)
    {
        if (0 <= x && x < 32 && 0 <= y && y < 32)
            set(x, y, c);
    }

    virtual void show() = 0;

    // Drawing API
    virtual void drawChar(int x, int y, char ch, RGB c) = 0;
    virtual void drawPixel(int x, int y, RGB c) = 0;
    virtual void drawLine(int x0, int y0, int x1, int y1, RGB c) = 0;
    virtual void drawRect(int x, int y, int w, int h, RGB c) = 0;
    virtual void drawCircle(int cx, int cy, int r, RGB c) = 0;
    virtual void fillRect(int x, int y, int w, int h, RGB c) = 0;
    virtual void fillCircle(int cx, int cy, int r, RGB c) = 0;

    // Text helpers
    virtual void advance() = 0;
    virtual void setCursor(int x, int y) = 0;
    virtual void setTextColor(RGB c) = 0;
    virtual void setTextSize(int s) = 0;
    virtual void print(char ch) = 0;
    virtual void print(const char *s) = 0;
    virtual void println(const char *s) = 0;

    virtual ~Matrix32() = default;
};

#endif // MATRIX32_H
