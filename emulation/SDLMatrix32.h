#ifndef SDL_MATRIX32_H
#define SDL_MATRIX32_H

#include "Matrix32.h"
#include "Helpers.h"
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

// Parameters that control how each logical LED cell is rendered on screen.
// - scale:  number of screen pixels per 1 matrix pixel
// - margin: black bezel around the LED inside the cell (in screen pixels)
// - fill:   diameter as a fraction of the inner area (0..1)
// - inner:  derived = scale - 2*margin
// - radius: derived = int(0.5 * inner * fill)
struct LEDcell
{
    int scale;
    int margin;
    float fill;
    int inner;
    int radius;
};

// SDL-backed implementation of Matrix32 for the desktop emulator.
// It supports two render modes:
// 1) Screen mode: fast blit of a 32x32 RGB texture
// 2) LED mode:    each pixel is drawn as a small colored circle in a black cell
class SDLMatrix32 : public Matrix32
{
public:
    // Construct an uninitialized instance. Call begin() before use.
    SDLMatrix32();
    // Destroys SDL resources (texture, renderer, window) and quits SDL.
    ~SDLMatrix32() override;

    // 32x32 RGB framebuffer (row-major)
    Color888 fb_[MATRIX_WIDTH * MATRIX_HEIGHT]{};
    // Convert (x,y) to framebuffer index.
    static constexpr int coordToIndex(int x, int y);
    Color888 get(int x, int y) const;

    // Initialize SDL window, renderer, streaming texture, and compute initial scale.
    void begin() override;
    // Get the underlying SDL_Window (for input provider).
    SDL_Window *window() const { return win_; }
    // Clear the 32x32 framebuffer to black.
    void clear() override;
    // Set a single framebuffer pixel (bounds-checked).
    void set(int x, int y, Color333 c) override;
    // Present the framebuffer using the current render mode (screen or LED).
    void show() override;

    // Toggle LED rendering mode.
    void toggleLEDMode() { led_mode_ = !led_mode_; }

    // Draw a 5x7 glyph scaled by setTextSize() at (x,y).
    void drawChar(int x, int y, char ch, Color333 c) override;
    // Set one pixel (alias for set()).
    void drawPixel(int x, int y, Color333 c) override;
    // Draw a line using Bresenham.
    void drawLine(int x0, int y0, int x1, int y1, Color333 c) override;
    // Draw an axis-aligned rectangle outline.
    void drawRect(int x, int y, int w, int h, Color333 c) override;
    // Draw a circle outline using the midpoint algorithm.
    void drawCircle(int cx, int cy, int r, Color333 c) override;
    // Fill an axis-aligned rectangle.
    void fillRect(int x, int y, int w, int h, Color333 c) override;
    // Fill a circle using horizontal spans.
    void fillCircle(int cx, int cy, int r, Color333 c) override;

    // Advance the text cursor by one glyph (including 1px spacing).
    void advance() override;
    // Set text cursor position in pixels.
    void setCursor(int x, int y) override;
    // Set text color for print/drawChar.
    void setTextColor(Color333 c) override;
    // Set integer text scale >= 1.
    void setTextSize(int s) override;
    // Print a single character (handles '\n').
    void print(char ch) override;
    // Print a C string.
    void print(const char *s) override;
    // Print a C string followed by newline.
    void println(const char *s) override;

    // Enable or query LED rendering mode.
    void setLEDMode(bool on) { led_mode_ = on; }
    bool ledMode() const { return led_mode_; }

    // Render one matrix pixel as an LED circle into the SDL renderer.
    void renderPixelAsLED(int x, int y, const LEDcell &cell);
    // Render the entire framebuffer as an LED matrix.
    void renderAsLEDMatrix();
    // Render the entire framebuffer as a blocky 32x32 screen.
    void renderAsScreen();

    // Converts Color333 to PixelColor
    Color888 convertColor(Color333 c);

private:
    // Text state
    int cx{0};                      // cursor x in pixels
    int cy{0};                      // cursor y in pixels
    int lineStartX{0};              // start-of-line x for newline handling
    int ts{1};                      // text scale
    Color333 tc{Color333{7, 7, 7}}; // text color

    // SDL state
    SDL_Window *win_{};
    SDL_Renderer *ren_{};
    SDL_Texture *tex_{};

    // Rendering options
    bool led_mode_{false};
    int scale_{16}; // screen pixels per logical LED

    // Low-level framebuffer helpers
    void span(int x0, int x1, int y, Color333 c); // clamped horizontal span
    void drawHLine(int x, int y, int w, Color333 c);
    void drawVLine(int x, int y, int h, Color333 c);
    void plot8(int cx, int cy, int x, int y, Color333 c); // 8-way circle symmetry plot
    void drawPixelScaled(int x, int y, Color333 c);       // draw ts x ts block

    // SDL helpers
    static inline void SetRGBA(SDL_Renderer *r, Intensity3 r8, Intensity3 g8, Intensity8 b8, Intensity8 a = 255);
    static void drawLEDAsCircle(SDL_Renderer *ren, int cx, int cy, int r);

    // Build LEDcell from current scale and chosen styling.
    LEDcell makeLEDcell() const;

    // Converts 0..7 -> 0..255 with rounding
    Intensity8 expand3to8(Intensity3 v)
    {
        // Adding half the divisor before dividing performs round‑to‑nearest: floor((v × 255 + 7/2) / 7)
        // Since we can’t add 3.5 in integers, we use +3 as a close, deterministic half
        return (Intensity8)((v * 255 + 3) / 7);
    }
};

#endif // SDL_MATRIX32_H
