#ifndef RGB_MATRIX_ADAPTER_H
#define RGB_MATRIX_ADAPTER_H

// Wrapper around the RGBmatrixPanel class for use

#include "Draw32.h"
#include <RGBmatrixPanel.h>

class RGBMatrixPanelAdapter : public Draw32 {
public:
	explicit RGBMatrixPanelAdapter(RGBmatrixPanel& panel) : m(panel) {}

	// drawPixel is 1:1
	void drawPixel(int x, int y, RGB c) override {
		m.drawPixel(x, y, color(c));
	}

	// Shapes map 1:1
	void drawLine(int x0, int y0, int x1, int y1, RGB c) override {
		m.drawLine(x0, y0, x1, y1, color(c));
	}
	void drawRect(int x, int y, int w, int h, RGB c) override {
		m.drawRect(x, y, w, h, color(c));
	}
	void drawCircle(int cx, int cy, int r, RGB c) override {
		m.drawCircle(cx, cy, r, color(c));
	}
	void fillRect(int x, int y, int w, int h, RGB c) override {
		m.fillRect(x, y, w, h, color(c));
	}
	void fillCircle(int cx, int cy, int r, RGB c) override {
		m.fillCircle(cx, cy, r, color(c));
	}

	// Text
	void setCursor(int x, int y) override { m.setCursor(x, y); }
	void setTextColor(RGB c) override { m.setTextColor(color(c)); }
	void setTextSize(int s) override { m.setTextSize(s); }
	void print(char ch) override { m.print(ch); }
	void print(const char* s) override { m.print(s); }
	void println(const char* s) override { m.println(s); }

private:
	inline uint16_t color(RGB c) const {
		auto to3 = [](uint8_t v) { return static_cast<uint8_t>((v * 7 + 127) / 255); };
		return m.Color333(to3(c.r), to3(c.g), to3(c.b));
	}
	RGBmatrixPanel& m;
};

#endif
