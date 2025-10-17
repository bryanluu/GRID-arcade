#include "SDLMatrix32.h"
#include <SDL.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

// ctor: create an empty object; call begin() before rendering
SDLMatrix32::SDLMatrix32() = default;

// dtor: release SDL resources safely
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

Color888 SDLMatrix32::convertColor(Color333 c)
{
    return Color888{expand3to8(c.r), expand3to8(c.g), expand3to8(c.b)};
}

// Convert (x,y) to framebuffer index.
constexpr int SDLMatrix32::coordToIndex(int x, int y)
{
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
        throw std::out_of_range("SDLMatrix32 pixel out of bounds");

    return y * MATRIX_WIDTH + x;
}

Color888 SDLMatrix32::get(int x, int y) const { return fb_[coordToIndex(x, y)]; }

// Initialize SDL window, renderer, and streaming texture. Also compute initial scale_.
void SDLMatrix32::begin()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(SDL_GetError());
    win_ = SDL_CreateWindow("GRID-Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 320, SDL_WINDOW_RESIZABLE);
    if (!win_)
        throw std::runtime_error(SDL_GetError());
    ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED);
    if (!ren_)
        throw std::runtime_error(SDL_GetError());
    tex_ = SDL_CreateTexture(ren_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, MATRIX_WIDTH, MATRIX_HEIGHT);
    if (!tex_)
        throw std::runtime_error(SDL_GetError());
    clear();
    recomputeScale();
    // Pump once so macOS shows the window promptly
    SDL_PumpEvents();
}

// Zero the framebuffer to black
void SDLMatrix32::clear()
{
    for (auto &p : fb_)
        p = {0, 0, 0};
}

// Set one pixel
void SDLMatrix32::set(int x, int y, Color333 c)
{
    fb_[coordToIndex(x, y)] = convertColor(c);
    if (immediate)
        show();
}

// Present using current render mode
void SDLMatrix32::show()
{
    if (!led_mode_)
    {
        renderAsScreen();
        return;
    }
    renderAsLEDMatrix();
}

// Draw one 5x7 glyph at (x,y), scaled by ts
void SDLMatrix32::drawChar(int x, int y, char ch, Color333 c)
{
    const PixelMap *glyph = FONT5x7[ch - ASCII_START];
    for (int col = 0; col < FONT_GLYPH_WIDTH; ++col)
    {
        PixelColumn bits = glyph[col];
        for (int row = 0; row < FONT_GLYPH_HEIGHT; ++row)
            if (bits & (1u << row))
                drawPixelScaled(x + col, y + row, c);
    }
    if (immediate)
        show();
}

// Alias for set()
void SDLMatrix32::drawPixel(int x, int y, Color333 c)
{
    setSafe(x, y, c);
    if (immediate)
        show();
}

// Bresenham line
void SDLMatrix32::drawLine(int x0, int y0, int x1, int y1, Color333 c)
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
    if (immediate)
        show();
}

// Rectangle outline
void SDLMatrix32::drawRect(int x, int y, int w, int h, Color333 c)
{
    if (w <= 0 || h <= 0)
        return;
    drawHLine(x, y, w, c);
    drawHLine(x, y + h - 1, w, c);
    drawVLine(x, y, h, c);
    drawVLine(x + w - 1, y, h, c);
    if (immediate)
        show();
}

// Midpoint circle outline
void SDLMatrix32::drawCircle(int cx, int cy, int r, Color333 c)
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
    if (immediate)
        show();
}

// Filled rectangle
void SDLMatrix32::fillRect(int x, int y, int w, int h, Color333 c)
{
    int x0 = std::max(0, x), y0 = std::max(0, y);
    int x1 = std::min(MATRIX_WIDTH - 1, x + w - 1), y1 = std::min(MATRIX_HEIGHT - 1, y + h - 1);
    for (int yy = y0; yy <= y1; ++yy)
        for (int xx = x0; xx <= x1; ++xx)
            setSafe(xx, yy, c);
    if (immediate)
        show();
}

// Filled circle via spans
void SDLMatrix32::fillCircle(int cx, int cy, int r, Color333 c)
{
    if (r < 0)
        return;
    int x = r, y = 0, err = 1 - r;
    while (x >= y)
    {
        span(cx - x, cx + x, cy + y, c);
        span(cx - x, cx + x, cy - y, c);
        span(cx - y, cx + y, cy + x, c);
        span(cx - y, cx + y, cy - x, c);
        ++y;
        if (err < 0)
            err += 2 * y + 1;
        else
        {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
    if (immediate)
        show();
}

// Move cursor by 1 glyph (5px + 1px spacing) at current scale
void SDLMatrix32::advance() { cx += ts * FONT_CHAR_WIDTH; }

// Set text cursor
void SDLMatrix32::setCursor(int x, int y)
{
    cx = x;
    cy = y;
    lineStartX = x;
}

// Set text color
void SDLMatrix32::setTextColor(Color333 c) { tc = c; }

// Set integer text scale >= 1
void SDLMatrix32::setTextSize(int s) { ts = std::max(1, s); }

// Print a single character (handles newline)
void SDLMatrix32::print(char ch)
{
    if (ch == '\n')
    {
        cx = lineStartX;
        cy += ts * FONT_CHAR_HEIGHT;
        return;
    }
    if (ch < ASCII_START)
    {
        advance();
        return;
    }
    drawChar(cx, cy, ch, tc);
    advance();
}

// Print a C string
void SDLMatrix32::print(const char *s)
{
    for (const char *p = s; *p; ++p)
        print(*p);
}

// Print a C string then newline
void SDLMatrix32::println(const char *s)
{
    print(s);
    print('\n');
}

// Minimal helper: recompute integer scale_ from current renderer output size
void SDLMatrix32::recomputeScale()
{
    int outW = 0, outH = 0;
    SDL_GetRendererOutputSize(ren_, &outW, &outH); // HiDPI-safe
    const int square = std::min(outW, outH);
    scale_ = std::max(1, square / MATRIX_WIDTH);

    // Centered offsets for LED canvas
    const int canvas = scale_ * MATRIX_WIDTH;
    ledOffsetX_ = (outW - canvas) / 2;
    ledOffsetY_ = (outH - canvas) / 2;
}

// Render one matrix pixel as a circular LED inside a black cell
void SDLMatrix32::renderPixelAsLED(int x, int y, const LEDcell &cell)
{
    const int sx = ledOffsetX_ + x * cell.scale;
    const int sy = ledOffsetY_ + y * cell.scale;

    // Bezel
    SetRGBA(ren_, 0, 0, 0, 255);
    SDL_Rect cellRect{sx, sy, cell.scale, cell.scale};
    SDL_RenderFillRect(ren_, &cellRect);

    // Center of LED
    const int cx_led = sx + cell.margin + cell.inner / 2;
    const int cy_led = sy + cell.margin + cell.inner / 2;

    // Color from framebuffer (dim "off" LED for dome look)
    const auto pix = fb_[coordToIndex(x, y)];

    Intensity8 r = pix.r, g = pix.g, b = pix.b;
    if ((r | g | b) == 0)
        r = g = b = 12;
    SetRGBA(ren_, r, g, b, 255);

    // Filled circle
    drawLEDAsCircle(ren_, cx_led, cy_led, cell.radius);
}

// Render the whole framebuffer as LEDs
void SDLMatrix32::renderAsLEDMatrix()
{
    // Turn off logical-size scaling so LED cells use real drawable pixels
    SDL_RenderSetLogicalSize(ren_, 0, 0); // disables logical size
    SDL_RenderSetIntegerScale(ren_, SDL_FALSE);

    // Make sure our integer scale_ and offsets are based on drawable size
    recomputeScale(); // also recomputeOffsets() if you followed earlier step

    SDL_RenderClear(ren_);
    SetRGBA(ren_, 0, 0, 0, 255);
    const LEDcell cell = makeLEDcell();
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
        for (int x = 0; x < MATRIX_WIDTH; ++x)
            renderPixelAsLED(x, y, cell);
    SDL_RenderPresent(ren_);
}

// Render the whole framebuffer as a 32x32 texture
void SDLMatrix32::renderAsScreen()
{
    // Use SDL’s logical-size pipeline for crisp, centered integer scaling
    SDL_RenderSetLogicalSize(ren_, MATRIX_WIDTH, MATRIX_HEIGHT);
    SDL_RenderSetIntegerScale(ren_, SDL_TRUE);
    void *pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(tex_, nullptr, &pixels, &pitch) == 0)
    {
        auto *dst = static_cast<uint8_t *>(pixels);
        const int width = MATRIX_WIDTH, height = MATRIX_HEIGHT, bpp = 3;
        for (int y = 0; y < height; ++y)
        {
            const uint8_t *src_row = reinterpret_cast<const uint8_t *>(&fb_[y * width]);
            std::memcpy(dst + y * pitch, src_row, width * bpp);
        }
        SDL_UnlockTexture(tex_);
    }
    SDL_RenderClear(ren_);
    SDL_RenderCopy(ren_, tex_, nullptr, nullptr); // NULL dst uses logical size
    SDL_RenderPresent(ren_);
}

// Draw a clamped horizontal span of pixels in the framebuffer
void SDLMatrix32::span(int x0, int x1, int y, Color333 c)
{
    if (y < 0 || y >= MATRIX_HEIGHT)
        return;
    x0 = std::max(0, x0);
    x1 = std::min(MATRIX_WIDTH - 1, x1);
    for (int x = x0; x <= x1; ++x)
        setSafe(x, y, c);
}

// Draw a horizontal line in the framebuffer
void SDLMatrix32::drawHLine(int x, int y, int w, Color333 c)
{
    for (int i = 0; i < w; ++i)
        setSafe(x + i, y, c);
}

// Draw a vertical line in the framebuffer
void SDLMatrix32::drawVLine(int x, int y, int h, Color333 c)
{
    for (int i = 0; i < h; ++i)
        setSafe(x, y + i, c);
}

// Plot using 8-way symmetry for circle algorithms
void SDLMatrix32::plot8(int cx, int cy, int x, int y, Color333 c)
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

// Draw a ts x ts block at logical (x,y)
void SDLMatrix32::drawPixelScaled(int x, int y, Color333 c)
{
    for (int dy = 0; dy < ts; ++dy)
        for (int dx = 0; dx < ts; ++dx)
            setSafe(x * ts + dx, y * ts + dy, c);
}

// Set renderer draw color (RGBA)
inline void SDLMatrix32::SetRGBA(SDL_Renderer *r, Intensity8 r8, Intensity8 g8, Intensity8 b8, Intensity8 a)
{
    SDL_SetRenderDrawColor(r, r8, g8, b8, a);
}

// Draw a filled circle via horizontal spans on the SDL renderer
static inline void HSpan(SDL_Renderer *ren, int x0, int x1, int y) { SDL_RenderDrawLine(ren, x0, y, x1, y); }
void SDLMatrix32::drawLEDAsCircle(SDL_Renderer *ren, int cx, int cy, int r)
{
    int x = r, y = 0, err = 1 - r;
    while (x >= y)
    {
        HSpan(ren, cx - x, cx + x, cy + y);
        HSpan(ren, cx - x, cx + x, cy - y);
        HSpan(ren, cx - y, cx + y, cy + x);
        HSpan(ren, cx - y, cx + y, cy - x);
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

// Build LEDcell parameters from current scale and styling constants
LEDcell SDLMatrix32::makeLEDcell() const
{
    LEDcell cell;
    cell.scale = scale_;
    cell.margin = 1;
    cell.fill = 0.70f;
    cell.inner = std::max(0, cell.scale - 2 * cell.margin);
    cell.radius = std::max(1, int(0.5f * cell.inner * cell.fill));
    return cell;
}

// v: 0..7  ->  0..255 with inverse-gamma to brighten low codes
const Intensity8 SDLMatrix32::kExpand3to8Gamma[8] =
    {0, 150, 181, 202, 220, 233, 245, 255};
