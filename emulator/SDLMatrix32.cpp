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

    // adjust scale as window is resized
    int w{0};
    int h{0};
    SDL_GetWindowSize(win_, &w, &h);
    scale_ = std::max(1, std::min(w, h) / 32); // integer pixels per cell
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

void SDLMatrix32::SetRGBA(SDL_Renderer *r, uint8_t r8, uint8_t g8, uint8_t b8, uint8_t a)
{
    SDL_SetRenderDrawColor(r, r8, g8, b8, a);
}

// Draw a filled circle centered at (cx, cy) with radius r
void SDLMatrix32::fillCircle(SDL_Renderer *ren, int cx, int cy, int r)
{
    int x = r, y = 0, err = 1 - r;
    auto hspan = [&](int x0, int x1, int y)
    {
        SDL_RenderDrawLine(ren, x0, y, x1, y);
    };
    while (x >= y)
    {
        hspan(cx - x, cx + x, cy + y);
        hspan(cx - x, cx + x, cy - y);
        hspan(cx - y, cx + y, cy + x);
        hspan(cx - y, cx + y, cy - x);
        ++y;
        if (err < 0)
            err += 2 * y + 1;
        else
        {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

/**
 * Renders a single pixel as an LED in the GRID
 */
void SDLMatrix32::renderPixelAsLED(int x, int y, LEDcell &cell)
{
    // Per-pixel screen rect origin
    const int sx = x * cell.scale;
    const int sy = y * cell.scale;

    // 1) draw black cell background (bezel)
    SetRGBA(ren_, 0, 0, 0, 255);
    SDL_Rect cellRect{sx, sy, cell.scale, cell.scale};
    SDL_RenderFillRect(ren_, &cellRect);

    // 2) compute LED center within cell with margin
    const int cx = sx + cell.margin + cell.inner / 2;
    const int cy = sy + cell.margin + cell.inner / 2;

    // 3) LED color from framebuffer
    // fb_ should be your RGB buffer, 3 bytes per pixel in row-major order.
    const SDLMatrix32::Pixel pixel = fb_[(y * 32 + x)];
    const uint8_t r = pixel.r;
    const uint8_t g = pixel.g;
    const uint8_t b = pixel.b;

    // 4) draw filled circle
    SetRGBA(ren_, r, g, b, 255);
    fillCircle(ren_, cx, cy, cell.radius);

    // Optional: specular highlight to mimic LED dome
    // Smaller white dot offset toward top-left.
    // Uncomment to taste:
    // SetRGBA(ren_, 255,255,255, 40);
    // fillCircle(ren_, cx - radius/3, cy - radius/3, std::max(1, radius/4));
}

/**
 * Renders GRID as a virtual LED matrix
 */
void SDLMatrix32::renderAsLEDMatrix()
{
    // LED mode: draw cells
    // Choose visual tuning:
    const int scale = scale_;  // e.g., 16 or 20; use what your window uses
    const int margin = 1;     // black border around each cell
    const float fill = 0.70f; // fraction of drawable inner size for diameter
    const int inner = scale - 2 * margin;
    const int radius = std::max(1, int(0.5f * inner * fill));
    LEDcell cell { scale, margin, fill, inner, radius };

    // Optional: background black (already default if you clear to black)
    SDL_RenderClear(ren_);
    SetRGBA(ren_, 0, 0, 0, 255);

    // Draw cells
    for (int y = 0; y < 32; ++y)
    {
        for (int x = 0; x < 32; ++x)
        {
            renderPixelAsLED(x, y, cell);
        }
    }

    SDL_RenderPresent(ren_);
}

/**
 * Renders GRID as a small screen with uniform block pixels
 */
void SDLMatrix32::renderAsScreen()
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

/**
 * Display the resulting GRID, as a screen or LED matrix if led_mode_ is on
 */
void SDLMatrix32::show()
{
    if (!led_mode_)
        renderAsScreen();
    else
        renderAsLEDMatrix();
}
