#include "App.h"

App::App(Matrix32& matrix) : m(matrix)
{
    m.begin();
    m.setTextSize(1);
}

void App::setup()
{
	m.fillRect(0,0,32,32, rgb(0,128,0));
	m.drawRect(0,0,32,32, rgb(255,255,0));
    m.drawPixel(31, 31, rgb(0, 0, 255));
	m.setCursor(1,0);
	m.setTextSize(1);
	m.setTextColor(rgb(255,255,255));
	m.println("GRID");
	m.show();
}

void App::loop(uint32_t now)
{
    // do nothing for now
}
