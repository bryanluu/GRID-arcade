#include "App.h"

App::App(Matrix32& matrix) : m(matrix) {}

void App::setup()
{
    m.begin();

    // draw a pixel in solid white
    m.drawPixel(0, 0, Color333{7, 7, 7});
    delay(500);

    // fix the screen with green
    m.fillRect(0, 0, 32, 32, Color333{0, 7, 0});
    delay(500);

    // draw a box in yellow
    m.drawRect(0, 0, 32, 32, Color333{7, 7, 0});
    delay(500);

    // draw an 'X' in red
    m.drawLine(0, 0, 31, 31, Color333{7, 0, 0});
    m.drawLine(31, 0, 0, 31, Color333{7, 0, 0});
    delay(500);

    // draw a blue circle
    m.drawCircle(10, 10, 10, Color333{0, 0, 7});
    delay(500);

    // fill a violet circle
    m.fillCircle(21, 21, 10, Color333{7, 0, 7});
    delay(500);

    m.clear();

    // draw some text!
    m.setCursor(1, 0); // start at top left, with one pixel of spacing
    m.setTextSize(1);  // size 1 == 8 pixels high
    // m.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    m.setTextColor(Color333{7, 7, 7});
    m.println(" Ada");
    m.println("fruit");

    // print each letter with a rainbow color
    m.setTextColor(Color333{7, 0, 0});
    m.print('3');
    m.setTextColor(Color333{7, 4, 0});
    m.print('2');
    m.setTextColor(Color333{7, 7, 0});
    m.print('x');
    m.setTextColor(Color333{4, 7, 0});
    m.print('3');
    m.setTextColor(Color333{0, 7, 0});
    m.println("2");

    m.setTextColor(Color333{0, 7, 7});
    m.print('*');
    m.setTextColor(Color333{0, 4, 7});
    m.print('R');
    m.setTextColor(Color333{0, 0, 7});
    m.print('G');
    m.setTextColor(Color333{4, 0, 7});
    m.print('B');
    m.setTextColor(Color333{7, 0, 4});
    m.print('*');

    // whew!
}

void App::loop(uint32_t now)
{
    // do nothing for now
}
