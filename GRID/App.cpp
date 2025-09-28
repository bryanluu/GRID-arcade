#include "App.h"

App::App(Matrix32& matrix) : matrix(matrix) {}

void App::setup()
{
    matrix.begin();

    // draw a pixel in solid white
    matrix.drawPixel(0, 0, Color333{7, 7, 7});
    delay(500);

    // fix the screen with green
    matrix.fillRect(0, 0, 32, 32, Color333{0, 7, 0});
    delay(500);

    // draw a box in yellow
    matrix.drawRect(0, 0, 32, 32, Color333{7, 7, 0});
    delay(500);

    // draw an 'X' in red
    matrix.drawLine(0, 0, 31, 31, Color333{7, 0, 0});
    matrix.drawLine(31, 0, 0, 31, Color333{7, 0, 0});
    delay(500);

    // draw a blue circle
    matrix.drawCircle(10, 10, 10, Color333{0, 0, 7});
    delay(500);

    // fill a violet circle
    matrix.fillCircle(21, 21, 10, Color333{7, 0, 7});
    delay(500);

    matrix.clear();

    // draw some text!
    matrix.setCursor(1, 0); // start at top left, with one pixel of spacing
    matrix.setTextSize(1);  // size 1 == 8 pixels high
    // matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    matrix.setTextColor(Color333{7, 7, 7});
    matrix.println(" Ada");
    matrix.println("fruit");

    // print each letter with a rainbow color
    matrix.setTextColor(Color333{7, 0, 0});
    matrix.print('3');
    matrix.setTextColor(Color333{7, 4, 0});
    matrix.print('2');
    matrix.setTextColor(Color333{7, 7, 0});
    matrix.print('x');
    matrix.setTextColor(Color333{4, 7, 0});
    matrix.print('3');
    matrix.setTextColor(Color333{0, 7, 0});
    matrix.println("2");

    matrix.setTextColor(Color333{0, 7, 7});
    matrix.print('*');
    matrix.setTextColor(Color333{0, 4, 7});
    matrix.print('R');
    matrix.setTextColor(Color333{0, 0, 7});
    matrix.print('G');
    matrix.setTextColor(Color333{4, 0, 7});
    matrix.print('B');
    matrix.setTextColor(Color333{7, 0, 4});
    matrix.print('*');

    // whew!
}

void App::loop(uint32_t now)
{
    // do nothing for now
}
