#ifndef DRAW32_H
#define DRAW32_H

#include "Matrix32.h"

// Cross-target drawing interface for 32x32, used by Scenes
struct Draw32 {
	virtual ~Draw32() = default;

	virtual void drawPixel(int x, int y, RGB c) = 0;

	// Shapes
	virtual void drawLine(int x0, int y0, int x1, int y1, RGB c) = 0;
	virtual void drawRect(int x, int y, int w, int h, RGB c) = 0;
	virtual void drawCircle(int cx, int cy, int r, RGB c) = 0;
	virtual void fillRect(int x, int y, int w, int h, RGB c) = 0;
	virtual void fillCircle(int cx, int cy, int r, RGB c) = 0;

	// Text
	virtual void setCursor(int x, int y) = 0;
	virtual void setTextColor(RGB c) = 0;
	virtual void setTextSize(int s) = 0; // integer scale (1 = 5x7)
	virtual void print(char ch) = 0;
	virtual void println(const char* s) = 0;
	virtual void print(const char* s) = 0;
};

#endif
