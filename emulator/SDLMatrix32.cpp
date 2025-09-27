#include "SDLMatrix32.h"
#include <SDL.h>
#include <cstring>
#include <stdexcept>

// The 5x7 pixel map for ASCII font (space..'~')
extern const uint8_t FONT5x7[96][5] = {
    /* ' ' */ {0x00, 0x00, 0x00, 0x00, 0x00},
    /* '!' */ {0x00, 0x00, 0x5F, 0x00, 0x00},
    /* '"' */ {0x00, 0x07, 0x00, 0x07, 0x00},
    /* '#' */ {0x14, 0x7F, 0x14, 0x7F, 0x14},
    /* '$' */ {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    /* '%' */ {0x23, 0x13, 0x08, 0x64, 0x62},
    /* '&' */ {0x36, 0x49, 0x55, 0x22, 0x50},
    /* '\' */ {0x00, 0x05, 0x03, 0x00, 0x00},
    /* '(' */ {0x00, 0x1C, 0x22, 0x41, 0x00},
    /* ')' */ {0x00, 0x41, 0x22, 0x1C, 0x00},
    /* '*' */ {0x14, 0x08, 0x3E, 0x08, 0x14},
    /* '+' */ {0x08, 0x08, 0x3E, 0x08, 0x08},
    /* ',' */ {0x00, 0x50, 0x30, 0x00, 0x00},
    /* '-' */ {0x08, 0x08, 0x08, 0x08, 0x08},
    /* '.' */ {0x00, 0x60, 0x60, 0x00, 0x00},
    /* '/' */ {0x20, 0x10, 0x08, 0x04, 0x02},
    /* '0' */ {0x3E, 0x51, 0x49, 0x45, 0x3E},
    /* '1' */ {0x00, 0x42, 0x7F, 0x40, 0x00},
    /* '2' */ {0x42, 0x61, 0x51, 0x49, 0x46},
    /* '3' */ {0x21, 0x41, 0x45, 0x4B, 0x31},
    /* '4' */ {0x18, 0x14, 0x12, 0x7F, 0x10},
    /* '5' */ {0x27, 0x45, 0x45, 0x45, 0x39},
    /* '6' */ {0x3C, 0x4A, 0x49, 0x49, 0x30},
    /* '7' */ {0x01, 0x71, 0x09, 0x05, 0x03},
    /* '8' */ {0x36, 0x49, 0x49, 0x49, 0x36},
    /* '9' */ {0x06, 0x49, 0x49, 0x29, 0x1E},
    /* ':' */ {0x00, 0x36, 0x36, 0x00, 0x00},
    /* ';' */ {0x00, 0x56, 0x36, 0x00, 0x00},
    /* '<' */ {0x08, 0x14, 0x22, 0x41, 0x00},
    /* '=' */ {0x14, 0x14, 0x14, 0x14, 0x14},
    /* '>' */ {0x00, 0x41, 0x22, 0x14, 0x08},
    /* '?' */ {0x02, 0x01, 0x51, 0x09, 0x06},
    /* '@' */ {0x32, 0x49, 0x79, 0x41, 0x3E},
    /* 'A' */ {0x7E, 0x11, 0x11, 0x11, 0x7E},
    /* 'B' */ {0x7F, 0x49, 0x49, 0x49, 0x36},
    /* 'C' */ {0x3E, 0x41, 0x41, 0x41, 0x22},
    /* 'D' */ {0x7F, 0x41, 0x41, 0x22, 0x1C},
    /* 'E' */ {0x7F, 0x49, 0x49, 0x49, 0x41},
    /* 'F' */ {0x7F, 0x09, 0x09, 0x09, 0x01},
    /* 'G' */ {0x3E, 0x41, 0x49, 0x49, 0x7A},
    /* 'H' */ {0x7F, 0x08, 0x08, 0x08, 0x7F},
    /* 'I' */ {0x00, 0x41, 0x7F, 0x41, 0x00},
    /* 'J' */ {0x20, 0x40, 0x41, 0x3F, 0x01},
    /* 'K' */ {0x7F, 0x08, 0x14, 0x22, 0x41},
    /* 'L' */ {0x7F, 0x40, 0x40, 0x40, 0x40},
    /* 'M' */ {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    /* 'N' */ {0x7F, 0x04, 0x08, 0x10, 0x7F},
    /* 'O' */ {0x3E, 0x41, 0x41, 0x41, 0x3E},
    /* 'P' */ {0x7F, 0x09, 0x09, 0x09, 0x06},
    /* 'Q' */ {0x3E, 0x41, 0x51, 0x21, 0x5E},
    /* 'R' */ {0x7F, 0x09, 0x19, 0x29, 0x46},
    /* 'S' */ {0x46, 0x49, 0x49, 0x49, 0x31},
    /* 'T' */ {0x01, 0x01, 0x7F, 0x01, 0x01},
    /* 'U' */ {0x3F, 0x40, 0x40, 0x40, 0x3F},
    /* 'V' */ {0x1F, 0x20, 0x40, 0x20, 0x1F},
    /* 'W' */ {0x3F, 0x40, 0x38, 0x40, 0x3F},
    /* 'X' */ {0x63, 0x14, 0x08, 0x14, 0x63},
    /* 'Y' */ {0x07, 0x08, 0x70, 0x08, 0x07},
    /* 'Z' */ {0x61, 0x51, 0x49, 0x45, 0x43},
    /* '[' */ {0x00, 0x7F, 0x41, 0x41, 0x00},
    /* '\' */ {0x02, 0x04, 0x08, 0x10, 0x20},
    /* ']' */ {0x00, 0x41, 0x41, 0x7F, 0x00},
    /* '^' */ {0x04, 0x02, 0x01, 0x02, 0x04},
    /* '_' */ {0x40, 0x40, 0x40, 0x40, 0x40},
    /* '`' */ {0x00, 0x01, 0x02, 0x04, 0x00},
    /* 'a' */ {0x20, 0x54, 0x54, 0x54, 0x78},
    /* 'b' */ {0x7F, 0x48, 0x44, 0x44, 0x38},
    /* 'c' */ {0x38, 0x44, 0x44, 0x44, 0x20},
    /* 'd' */ {0x38, 0x44, 0x44, 0x48, 0x7F},
    /* 'e' */ {0x38, 0x54, 0x54, 0x54, 0x18},
    /* 'f' */ {0x08, 0x7E, 0x09, 0x01, 0x02},
    /* 'g' */ {0x0C, 0x52, 0x52, 0x52, 0x3E},
    /* 'h' */ {0x7F, 0x08, 0x04, 0x04, 0x78},
    /* 'i' */ {0x00, 0x44, 0x7D, 0x40, 0x00},
    /* 'j' */ {0x20, 0x40, 0x44, 0x3D, 0x00},
    /* 'k' */ {0x7F, 0x10, 0x28, 0x44, 0x00},
    /* 'l' */ {0x00, 0x41, 0x7F, 0x40, 0x00},
    /* 'm' */ {0x7C, 0x04, 0x18, 0x04, 0x78},
    /* 'n' */ {0x7C, 0x08, 0x04, 0x04, 0x78},
    /* 'o' */ {0x38, 0x44, 0x44, 0x44, 0x38},
    /* 'p' */ {0x7C, 0x14, 0x14, 0x14, 0x08},
    /* 'q' */ {0x08, 0x14, 0x14, 0x18, 0x7C},
    /* 'r' */ {0x7C, 0x08, 0x04, 0x04, 0x08},
    /* 's' */ {0x48, 0x54, 0x54, 0x54, 0x20},
    /* 't' */ {0x04, 0x3F, 0x44, 0x40, 0x20},
    /* 'u' */ {0x3C, 0x40, 0x40, 0x20, 0x7C},
    /* 'v' */ {0x1C, 0x20, 0x40, 0x20, 0x1C},
    /* 'w' */ {0x3C, 0x40, 0x30, 0x40, 0x3C},
    /* 'x' */ {0x44, 0x28, 0x10, 0x28, 0x44},
    /* 'y' */ {0x0C, 0x50, 0x50, 0x50, 0x3C},
    /* 'z' */ {0x44, 0x64, 0x54, 0x4C, 0x44},
    /* '{' */ {0x00, 0x08, 0x36, 0x41, 0x00},
    /* '|' */ {0x00, 0x00, 0x7F, 0x00, 0x00},
    /* '}' */ {0x00, 0x41, 0x36, 0x08, 0x00},
    /* '~' */ {0x10, 0x08, 0x08, 0x10, 0x08}};

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

void SDLMatrix32::drawChar(int x, int y, char ch, RGB c)
{
    const uint8_t *glyph = FONT5x7[ch - 32];
    for (int col = 0; col < 5; ++col)
    {
        uint8_t bits = glyph[col];
        for (int row = 0; row < 7; ++row)
            if (bits & (1u << row))
                drawPixelScaled(x + col, y + row, c);
    }
}

void SDLMatrix32::advance() { cx += ts * 6; } // 5px glyph + 1px spacing

void SDLMatrix32::drawPixel(int x, int y, RGB c)
{
    setSafe(x, y, c);
}

void SDLMatrix32::drawLine(int x0, int y0, int x1, int y1, RGB c)
{
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true)
    {
        setSafe(x0, y0, c);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void SDLMatrix32::drawRect(int x, int y, int w, int h, RGB c)
{
    if (w <= 0 || h <= 0)
        return;
    drawHLine(x, y, w, c);
    drawHLine(x, y + h - 1, w, c);
    drawVLine(x, y, h, c);
    drawVLine(x + w - 1, y, h, c);
}

void SDLMatrix32::drawCircle(int cx, int cy, int r, RGB c)
{
    if (r < 0)
        return;
    int x = r, y = 0, err = 1 - r;
    while (x >= y)
    {
        plot8(cx, cy, x, y, c);
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

void SDLMatrix32::fillRect(int x, int y, int w, int h, RGB c)
{
    int x0 = std::max(0, x);
    int y0 = std::max(0, y);
    int x1 = std::min(31, x + w - 1);
    int y1 = std::min(31, y + h - 1);
    for (int yy = y0; yy <= y1; ++yy)
        for (int xx = x0; xx <= x1; ++xx)
            set(xx, yy, c);
}

void SDLMatrix32::fillCircle(int cx, int cy, int r, RGB c)
{
    if (r < 0)
        return;
    int x = r, y = 0, err = 1 - r;
    while (x >= y)
    {
        // draw horizontal spans between symmetric points
        span(cx - x, cx + x, cy + y, c);
        span(cx - x, cx + x, cy - y, c);
        span(cx - y, cx + y, cy + x, c);
        span(cx - y, cx + y, cy - x, c);
        ++y;
        if (err < 0)
        {
            err += 2 * y + 1;
        }
        else
        {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

void SDLMatrix32::setCursor(int x, int y)
{
    cx = x;
    cy = y;
    lineStartX = x;
}
void SDLMatrix32::setTextColor(RGB c) { tc = c; }
void SDLMatrix32::setTextSize(int s) { ts = std::max(1, s); }

void SDLMatrix32::print(char ch)
{
    if (ch == '\n')
    {
        cx = lineStartX;
        cy += ts * 8;
        return;
    }
    if (ch < 32)
    {
        advance();
        return;
    }
    drawChar(cx, cy, ch, tc);
    advance();
}
void SDLMatrix32::print(const char *s)
{
    for (const char *p = s; *p; ++p)
        print(*p);
}
void SDLMatrix32::println(const char *s)
{
    print(s);
    print('\n');
}

void SDLMatrix32::SetRGBA(SDL_Renderer *r, uint8_t r8, uint8_t g8, uint8_t b8, uint8_t a)
{
    SDL_SetRenderDrawColor(r, r8, g8, b8, a);
}

void SDLMatrix32::drawLEDAsCircle(SDL_Renderer *ren, int cx, int cy, int r)
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

// Draw a clamped horizontal span [x0..x1] at row y
void SDLMatrix32::span(int x0, int x1, int y, RGB c)
{
    if (y < 0 || y >= 32)
        return;
    x0 = std::max(0, x0);
    x1 = std::min(31, x1);
    for (int x = x0; x <= x1; ++x)
        setSafe(x, y, c);
}

void SDLMatrix32::drawHLine(int x, int y, int w, RGB c)
{
    for (int i = 0; i < w; ++i)
        setSafe(x + i, y, c);
}

void SDLMatrix32::drawVLine(int x, int y, int h, RGB c)
{
    for (int i = 0; i < h; ++i)
        setSafe(x, y + i, c);
}

void SDLMatrix32::plot8(int cx, int cy, int x, int y, RGB c)
{
    setSafe(cx + x, cy + y, c);
    setSafe(cx - x, cy + y, c);
    setSafe(cx + x, cy - y, c);
    setSafe(cx - x, cy - y, c);
    setSafe(cx + y, cy + x, c);
    setSafe(cx - y, cy + x, c);
    setSafe(cx + y, cy - x, c);
    setSafe(cx - y, cy - x, c);
}

void SDLMatrix32::drawPixelScaled(int x, int y, RGB c)
{
    for (int dy = 0; dy < ts; ++dy)
        for (int dx = 0; dx < ts; ++dx)
            setSafe(x * ts + dx, y * ts + dy, c);
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
    SetRGBA(ren_, 0, 0, 0);
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
    drawLEDAsCircle(ren_, cx, cy, cell.radius);

    // Optional: specular highlight to mimic LED dome
    // Smaller white dot offset toward top-left.
    // Uncomment to taste:
    // SetRGBA(ren_, 255,255,255, 40);
    // drawLEDAsCircle(ren_, cx - radius/3, cy - radius/3, std::max(1, radius/4));
}

/**
 * Renders GRID as a virtual LED matrix
 */
void SDLMatrix32::renderAsLEDMatrix()
{
    // LED mode: draw cells
    // Choose visual tuning:
    const int scale = scale_; // e.g., 16 or 20; use what your window uses
    const int margin = 1;     // black border around each cell
    const float fill = 0.70f; // fraction of drawable inner size for diameter
    const int inner = scale - 2 * margin;
    const int radius = std::max(1, int(0.5f * inner * fill));
    LEDcell cell{scale, margin, fill, inner, radius};

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
