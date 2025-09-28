#include "ExampleScene.h"
#include "Colors.h"

void ExampleScene::start()
{
    Matrix32& m = app->matrix;
    m.begin();

    // draw a pixel in solid white
    m.drawPixel(0, 0, WHITE);
    delay(500);

    // fix the screen with green
    m.fillRect(0, 0, 32, 32, GREEN);
    delay(500);

    // draw a box in yellow
    m.drawRect(0, 0, 32, 32, YELLOW);
    delay(500);

    // draw an 'X' in red
    m.drawLine(0, 0, 31, 31, RED);
    m.drawLine(31, 0, 0, 31, RED);
    delay(500);

    // draw a blue circle
    m.drawCircle(10, 10, 10, BLUE);
    delay(500);

    // fill a violet circle
    m.fillCircle(21, 21, 10, VIOLET);
    delay(500);

    m.clear();

    // draw some text!
    m.setCursor(1, 0); // start at top left, with one pixel of spacing
    m.setTextSize(1);  // size 1 == 8 pixels high
    // m.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    m.setTextColor(WHITE);
    m.println(" Ada");
    m.println("fruit");

    // print each letter with a rainbow color
    m.setTextColor(RED);
    m.print('3');
    m.setTextColor(ORANGE);
    m.print('2');
    m.setTextColor(YELLOW);
    m.print('x');
    m.setTextColor(LIME);
    m.print('3');
    m.setTextColor(GREEN);
    m.println("2");

    m.setTextColor(CYAN);
    m.print('*');
    m.setTextColor(AZURE);
    m.print('R');
    m.setTextColor(BLUE);
    m.print('G');
    m.setTextColor(PURPLE);
    m.print('B');
    m.setTextColor(PINK);
    m.print('*');

    // whew!
}

void ExampleScene::run()
{
    // Do nothing -- image doesn't change
}
