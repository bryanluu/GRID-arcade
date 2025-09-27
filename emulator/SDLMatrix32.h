#ifndef SDL_MATRIX32_H
#define SDL_MATRIX32_H

#include "Matrix32.h"

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

struct LEDcell
{
    const int scale;
    const int margin;
    const float fill;
    const int inner;
    const int radius;
};

// Declaration only; implementation in src/sdl_matrix32.cpp
class SDLMatrix32 : public Matrix32
{
public:
    SDLMatrix32();
    ~SDLMatrix32() override;

    void begin() override;
    void clear() override;
    void set(int x, int y, RGB c) override;
    void show() override;

    void drawChar(int x, int y, char ch, RGB c) override;
    void drawPixel(int x, int y, RGB c) override;
    void drawLine(int x0, int y0, int x1, int y1, RGB c) override;
    void drawRect(int x, int y, int w, int h, RGB c) override;
    void drawCircle(int cx, int cy, int r, RGB c) override;
    void fillRect(int x, int y, int w, int h, RGB c) override;
    void fillCircle(int cx, int cy, int r, RGB c) override;
    
    void advance() override;
    void setCursor(int x, int y) override;
    void setTextColor(RGB c) override;
    void setTextSize(int s) override;

    void print(char ch) override;
    void print(const char *s) override;
    void println(const char *s) override;

    void setLedMode(bool on) { led_mode_ = on; }
    bool ledMode() const { return led_mode_; }
    void renderPixelAsLED(int x, int y, LEDcell &cell);
    void renderAsLEDMatrix();
    void renderAsScreen();

private:
    int cx{0};
    int cy{0};
    int lineStartX{0};
    int ts{1}; // scale
    RGB tc{rgb(255, 255, 255)};
    SDL_Window *win_{};
    SDL_Renderer *ren_{};
    SDL_Texture *tex_{};
    bool led_mode_{false};
    int scale_{16};

    // 32x32 RGB buffer (row-major)
    struct Pixel
    {
        uint8_t r, g, b;
    };
    Pixel fb_[32 * 32]{};

	// Draw a clamped horizontal span [x0..x1] at row y
    void span(int x0, int x1, int y, RGB c);
	void drawHLine(int x, int y, int w, RGB c);
	void drawVLine(int x, int y, int h, RGB c);
    void plot8(int cx, int cy, int x, int y, RGB c);
    void drawPixelScaled(int x, int y, RGB c);
    void SetRGBA(SDL_Renderer *r, uint8_t r8, uint8_t g8, uint8_t b8, uint8_t a = 255);
    void drawLEDAsCircle(SDL_Renderer *ren, int cx, int cy, int r);
};

#endif
