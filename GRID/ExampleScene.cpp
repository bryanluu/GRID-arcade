#include "ExampleScene.h"
#include "Colors.h"
#include "helpers.h" // for delay()

void ExampleScene::setup(Matrix32 &gfx)
{
    // draw a pixel in solid white
    gfx.drawPixel(0, 0, WHITE);
    delay(500);

    // fix the screen with green
    gfx.fillRect(0, 0, 32, 32, GREEN);
    delay(500);

    // draw a box in yellow
    gfx.drawRect(0, 0, 32, 32, YELLOW);
    delay(500);

    // draw an 'X' in red
    gfx.drawLine(0, 0, 31, 31, RED);
    gfx.drawLine(31, 0, 0, 31, RED);
    delay(500);

    // draw a blue circle
    gfx.drawCircle(10, 10, 10, BLUE);
    delay(500);

    // fill a violet circle
    gfx.fillCircle(21, 21, 10, VIOLET);
    delay(500);

    gfx.clear();

    // draw some text!
    gfx.setCursor(1, 0); // start at top left, with one pixel of spacing
    gfx.setTextSize(1);  // size 1 == 8 pixels high
    // gfx.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    gfx.setTextColor(WHITE);
    gfx.println(" Ada");
    gfx.println("fruit");

    // print each letter with a rainbow color
    gfx.setTextColor(RED);
    gfx.print('3');
    gfx.setTextColor(ORANGE);
    gfx.print('2');
    gfx.setTextColor(YELLOW);
    gfx.print('x');
    gfx.setTextColor(LIME);
    gfx.print('3');
    gfx.setTextColor(GREEN);
    gfx.println("2");

    gfx.setTextColor(CYAN);
    gfx.print('*');
    gfx.setTextColor(AZURE);
    gfx.print('R');
    gfx.setTextColor(BLUE);
    gfx.print('G');
    gfx.setTextColor(PURPLE);
    gfx.print('B');
    gfx.setTextColor(PINK);
    gfx.print('*');

    // whew!
}

void ExampleScene::loop(Matrix32 &gfx, millis_t dt)
{
    // Do nothing -- image doesn't change
}
