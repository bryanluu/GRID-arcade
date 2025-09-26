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

private:
    SDL_Window *win_{};
    SDL_Renderer *ren_{};
    SDL_Texture *tex_{};

    // 32x32 RGB buffer (row-major)
    struct Pixel
    {
        uint8_t r, g, b;
    };
    Pixel fb_[32 * 32]{};
};

#endif
