#ifndef MATRIX32_ADAPTER_H
#define MATRIX32_ADAPTER_H

#include "Draw32.h"
#include <algorithm>
#include <cstdlib>

// 5x7 ASCII font (space..'~'), defined in font5x7.cpp
extern const uint8_t FONT5x7[96][5];

class Matrix32Adapter : public Draw32 {
private:
	// Draw a clamped horizontal span [x0..x1] at row y
    void span(int x0, int x1, int y, RGB c) {
        if (y < 0 || y >= 32) return;
        x0 = std::max(0, x0);
        x1 = std::min(31, x1);
        for (int x = x0; x <= x1; ++x) setSafe(x, y, c);
    }

public:
	explicit Matrix32Adapter(Matrix32& m) : mtx(m) {}

    void drawPixel(int x, int y, RGB c) override {
        setSafe(x, y, c);
    }

	void drawLine(int x0, int y0, int x1, int y1, RGB c) override {
		int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;
		while (true) {
			setSafe(x0, y0, c);
			if (x0 == x1 && y0 == y1) break;
			int e2 = 2 * err;
			if (e2 >= dy) { err += dy; x0 += sx; }
			if (e2 <= dx) { err += dx; y0 += sy; }
		}
	}

	void drawRect(int x, int y, int w, int h, RGB c) override {
		if (w <= 0 || h <= 0) return;
		drawHLine(x, y, w, c);
		drawHLine(x, y + h - 1, w, c);
		drawVLine(x, y, h, c);
		drawVLine(x + w - 1, y, h, c);
	}

	void drawCircle(int cx, int cy, int r, RGB c) override {
		if (r < 0) return;
		int x = r, y = 0, err = 1 - r;
		while (x >= y) {
			plot8(cx, cy, x, y, c);
			++y;
			if (err < 0) err += 2*y + 1; else { --x; err += 2*(y - x) + 1; }
		}
	}

	void fillRect(int x, int y, int w, int h, RGB c) override {
		int x0 = std::max(0, x);
		int y0 = std::max(0, y);
		int x1 = std::min(31, x + w - 1);
		int y1 = std::min(31, y + h - 1);
		for (int yy = y0; yy <= y1; ++yy)
			for (int xx = x0; xx <= x1; ++xx)
				mtx.set(xx, yy, c);
	}

	void fillCircle(int cx, int cy, int r, RGB c) override {
		if (r < 0) return;
		int x = r, y = 0, err = 1 - r;
		while (x >= y) {
			// draw horizontal spans between symmetric points
			span(cx - x, cx + x, cy + y, c);
			span(cx - x, cx + x, cy - y, c);
			span(cx - y, cx + y, cy + x, c);
			span(cx - y, cx + y, cy - x, c);
			++y;
			if (err < 0) {
				err += 2*y + 1;
			} else {
				--x;
				err += 2*(y - x) + 1;
			}
		}
	}

	void setCursor(int x, int y) override { cx = x; cy = y; lineStartX = x; }
	void setTextColor(RGB c) override { tc = c; }
	void setTextSize(int s) override { ts = std::max(1, s); }

	void print(char ch) override {
		if (ch == '\n') { cx = lineStartX; cy += ts * 8; return; }
		if (ch < 32) { advance(); return; }
		drawChar(cx, cy, ch, tc);
		advance();
	}
	void print(const char* s) override { for (const char* p = s; *p; ++p) print(*p); }
	void println(const char* s) override { print(s); print('\n'); }

private:
	Matrix32& mtx;
	int cx = 0, cy = 0, lineStartX = 0;
	int ts = 1; // scale
	RGB tc = rgb(255,255,255);

	void setSafe(int x, int y, RGB c) {
		if (0 <= x && x < 32 && 0 <= y && y < 32) mtx.set(x, y, c);
	}
	void drawHLine(int x, int y, int w, RGB c) { for (int i = 0; i < w; ++i) setSafe(x + i, y, c); }
	void drawVLine(int x, int y, int h, RGB c) { for (int i = 0; i < h; ++i) setSafe(x, y + i, c); }
	void plot8(int cx, int cy, int x, int y, RGB c) {
		setSafe(cx + x, cy + y, c); setSafe(cx - x, cy + y, c);
		setSafe(cx + x, cy - y, c); setSafe(cx - x, cy - y, c);
		setSafe(cx + y, cy + x, c); setSafe(cx - y, cy + x, c);
		setSafe(cx + y, cy - x, c); setSafe(cx - y, cy - x, c);
	}
	void drawChar(int x, int y, char ch, RGB c) {
		const uint8_t* glyph = FONT5x7[ch - 32];
		for (int col = 0; col < 5; ++col) {
			uint8_t bits = glyph[col];
			for (int row = 0; row < 7; ++row)
				if (bits & (1u << row)) drawPixelScaled(x + col, y + row, c);
		}
	}
	void drawPixelScaled(int x, int y, RGB c) {
		for (int dy = 0; dy < ts; ++dy)
			for (int dx = 0; dx < ts; ++dx)
				setSafe(x*ts + dx, y*ts + dy, c);
	}
	void advance() { cx += ts * 6; } // 5px glyph + 1px spacing
};

#endif
