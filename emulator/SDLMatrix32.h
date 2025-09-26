#ifndef SDL_MATRIX32_H
#define SDL_MATRIX32_H

#include "Matrix32.h"

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

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

    void setLedMode(bool on) { led_mode_ = on; }
    bool ledMode() const { return led_mode_; }

private:
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

    static void SetRGBA(SDL_Renderer *r, uint8_t r8, uint8_t g8, uint8_t b8, uint8_t a = 255);
    static void fillCircle(SDL_Renderer *ren, int cx, int cy, int r);
};

#endif
