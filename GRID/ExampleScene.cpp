#include "ExampleScene.h"
#include "Matrix32.h"
#include "Colors.h"
#include "helpers.h"

void ExampleScene::setup(AppContext &ctx)
{
    // draw a pixel in solid white
    ctx.gfx.drawPixel(0, 0, WHITE);
    Helpers::sleep(500);

    // fix the screen with green
    ctx.gfx.fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, GREEN);
    Helpers::sleep(500);

    // draw a box in yellow
    ctx.gfx.drawRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, YELLOW);
    Helpers::sleep(500);

    // draw an 'X' in red
    ctx.gfx.drawLine(0, 0, MATRIX_WIDTH - 1, MATRIX_HEIGHT - 1, RED);
    ctx.gfx.drawLine(MATRIX_WIDTH - 1, 0, 0, MATRIX_HEIGHT - 1, RED);
    Helpers::sleep(500);

    // draw a blue circle
    ctx.gfx.drawCircle(10, 10, 10, BLUE);
    Helpers::sleep(500);

    // fill a violet circle
    ctx.gfx.fillCircle(21, 21, 10, VIOLET);
    Helpers::sleep(500);

    ctx.gfx.clear();

    // draw some text!
    ctx.gfx.setCursor(1, 0); // start at top left, with one pixel of spacing
    ctx.gfx.setTextSize(1);  // size 1 == 8 pixels high
    // ctx.gfx.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    ctx.gfx.setTextColor(WHITE);
    ctx.gfx.println(" Ada");
    ctx.gfx.println("fruit");

    // print each letter with a rainbow color
    ctx.gfx.setTextColor(RED);
    ctx.gfx.print('3');
    ctx.gfx.setTextColor(ORANGE);
    ctx.gfx.print('2');
    ctx.gfx.setTextColor(YELLOW);
    ctx.gfx.print('x');
    ctx.gfx.setTextColor(LIME);
    ctx.gfx.print('3');
    ctx.gfx.setTextColor(GREEN);
    ctx.gfx.println("2");

    ctx.gfx.setTextColor(CYAN);
    ctx.gfx.print('*');
    ctx.gfx.setTextColor(AZURE);
    ctx.gfx.print('R');
    ctx.gfx.setTextColor(BLUE);
    ctx.gfx.print('G');
    ctx.gfx.setTextColor(PURPLE);
    ctx.gfx.print('B');
    ctx.gfx.setTextColor(PINK);
    ctx.gfx.print('*');

    // whew!
}

void ExampleScene::loop(AppContext &ctx)
{
    // Do nothing -- image doesn't change
}
