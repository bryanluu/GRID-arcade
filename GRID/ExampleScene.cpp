#include "ExampleScene.h"
#include "Matrix32.h"
#include "Colors.h"
#include "Helpers.h"

void ExampleScene::setup(AppContext &ctx)
{
    // draw a pixel in solid white
    ctx.gfx.drawPixel(0, 0, Colors::Bright::White);
    ctx.time.sleep(500);

    // fix the screen with green
    ctx.gfx.fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, Colors::Bright::Green);
    ctx.time.sleep(500);

    // draw a box in yellow
    ctx.gfx.drawRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, Colors::Bright::Yellow);
    ctx.time.sleep(500);

    // draw an 'X' in red
    ctx.gfx.drawLine(0, 0, MATRIX_WIDTH - 1, MATRIX_HEIGHT - 1, Colors::Bright::Red);
    ctx.gfx.drawLine(MATRIX_WIDTH - 1, 0, 0, MATRIX_HEIGHT - 1, Colors::Bright::Red);
    ctx.time.sleep(500);

    // draw a blue circle
    ctx.gfx.drawCircle(10, 10, 10, Colors::Bright::Blue);
    ctx.time.sleep(500);

    // fill a violet circle
    ctx.gfx.fillCircle(21, 21, 10, Colors::Bright::Violet);
    ctx.time.sleep(500);

    ctx.gfx.clear();

    // draw some text!
    ctx.gfx.setCursor(1, 0); // start at top left, with one pixel of spacing
    ctx.gfx.setTextSize(1);  // size 1 == 8 pixels high
    // ctx.gfx.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    ctx.gfx.setTextColor(Colors::Bright::White);
    ctx.gfx.println(" Ada");
    ctx.gfx.println("fruit");

    // print each letter with a rainbow color
    ctx.gfx.setTextColor(Colors::Bright::Red);
    ctx.gfx.print('3');
    ctx.gfx.setTextColor(Colors::Bright::Orange);
    ctx.gfx.print('2');
    ctx.gfx.setTextColor(Colors::Bright::Yellow);
    ctx.gfx.print('x');
    ctx.gfx.setTextColor(Colors::Bright::Lime);
    ctx.gfx.print('3');
    ctx.gfx.setTextColor(Colors::Bright::Green);
    ctx.gfx.println("2");

    ctx.gfx.setTextColor(Colors::Bright::Cyan);
    ctx.gfx.print('*');
    ctx.gfx.setTextColor(Colors::Bright::Azure);
    ctx.gfx.print('R');
    ctx.gfx.setTextColor(Colors::Bright::Blue);
    ctx.gfx.print('G');
    ctx.gfx.setTextColor(Colors::Bright::Purple);
    ctx.gfx.print('B');
    ctx.gfx.setTextColor(Colors::Bright::Pink);
    ctx.gfx.print('*');

    // whew!
}

void ExampleScene::loop(AppContext &ctx)
{
    // Do nothing -- image doesn't change
}
