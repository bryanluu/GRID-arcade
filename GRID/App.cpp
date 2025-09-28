#include "App.h"

App::App(Matrix32& matrix) : m(matrix)
{
    m.begin();
    m.setTextSize(1);
}

void App::setup()
{
	m.fillRect(0,0,32,32, Color333{0,4,0});
	m.drawRect(0,0,32,32, Color333{7,7,0});
    m.drawPixel(31, 31, Color333{0,0,7});
	m.setCursor(1,0);
	m.setTextSize(1);
	m.setTextColor(Color333{7,7,7});
	m.println("GRID");
	m.show();
}

void App::loop(uint32_t now)
{
    // do nothing for now
}
