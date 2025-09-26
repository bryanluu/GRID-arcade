#include "SDLMatrix32.h"
#include <SDL.h>
#include <cstring>
#include <stdexcept>

SDLMatrix32::SDLMatrix32() = default;
SDLMatrix32::~SDLMatrix32()
{
    if (tex_)
        SDL_DestroyTexture(tex_);
    if (ren_)
        SDL_DestroyRenderer(ren_);
    if (win_)
        SDL_DestroyWindow(win_);
    SDL_Quit();
}

void SDLMatrix32::begin()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(SDL_GetError());
    win_ = SDL_CreateWindow("GRID-Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 320, SDL_WINDOW_RESIZABLE);
    if (!win_)
        throw std::runtime_error(SDL_GetError());
    ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren_)
        throw std::runtime_error(SDL_GetError());
    tex_ = SDL_CreateTexture(ren_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 32, 32);
    if (!tex_)
        throw std::runtime_error(SDL_GetError());
    clear();
}

void SDLMatrix32::clear()
{
    for (auto &p : fb_)
    {
        p = {0, 0, 0}; // set all pixels to black
    }
}

/**
 * Sets the SDL matrix's pixel at (x, y) to the color c
 */
void SDLMatrix32::set(int x, int y, RGB c)
{
    if ((unsigned)x < 32u && (unsigned)y < 32u)
    {
        fb_[y * 32 + x] = {c.r, c.g, c.b};
    }
}

void SDLMatrix32::show()
{
    void *pixels;
    int pitch;
    SDL_LockTexture(tex_, nullptr, &pixels, &pitch);
    auto *dst = static_cast<uint8_t *>(pixels);
    for (int y = 0; y < 32; y++)
    {
        std::memcpy(dst + y * pitch, &fb_[y * 32], 32 * 3);
    }
    SDL_UnlockTexture(tex_);
    SDL_RenderClear(ren_);
    SDL_RenderCopy(ren_, tex_, nullptr, nullptr);
    SDL_RenderPresent(ren_);
}
