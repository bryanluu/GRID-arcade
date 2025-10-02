#ifndef MATRIX32_H
#define MATRIX32_H

#include "Colors.h"
#include <cstdint>

#define MATRIX_SIZE     32
#define MATRIX_WIDTH    32
#define MATRIX_HEIGHT   32
#define ASCII_START     32

using MatrixPosition = uint8_t;
using PixelMap = uint8_t;
using PixelColumn = uint8_t;

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

    virtual ~Matrix32() = default;
protected:
    bool immediate{false}; // if true, show() after each draw operation
};

#endif // MATRIX32_H
