// Using the
// testshapes demo for Adafruit RGBmatrixPanel library.
// Demonstrates the drawing abilities of the RGBmatrixPanel library.
// For 32x32 RGB LED matrix:
// http://www.adafruit.com/products/607
// 32x32 MATRICES DO NOT WORK WITH ARDUINO UNO or METRO 328.

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include "App.h"
#include "Scene.h"
#include "RGBMatrix32.h"
#include <RGBmatrixPanel.h>

// Most of the signal pins are configurable, but the CLK pin has some
// special constraints.  On 8-bit AVR boards it must be on PORTB...
// Pin 11 works on the Arduino Mega.  On 32-bit SAMD boards it must be
// on the same PORT as the RGB data pins (D2-D7)...
// Pin 8 works on the Adafruit Metro M0 or Arduino Zero,
// Pin A4 works on the Adafruit Metro M4 (if using the Adafruit RGB
// Matrix Shield, cut trace between CLK pads and run a wire to A4).

#define CLK  8   // USE THIS ON ADAFRUIT METRO M0, etc.
//#define CLK A4 // USE THIS ON METRO M4 (not M0)
//#define CLK 11 // USE THIS ON ARDUINO MEGA
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3

RGBmatrixPanel panel(A, B, C, D, CLK, LAT, OE, false);
RGBMatrix32 matrix(panel);
App app(matrix);

class ExampleScene : public Scene
{
public:
    ExampleScene() : Scene()
    {
        m_name = "Example";
    }

    void start();
    void run();
};

void ExampleScene::start()
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

void ExampleScene::run()
{
    // Do nothing -- image doesn't change
}

ExampleScene exampleScene = ExampleScene();

void smokeTest()
{
    matrix.begin();

    // Solid green
    matrix.fillRect(0, 0, 32, 32, Color333{0, 7, 0});
    delay(2000);

    // Alternating rows
    matrix.clear();
    for (int y = 0; y < 32; ++y)
    {
        for (int x = 0; x < 32; ++x)
        {
            matrix.drawPixel(x, y, (y & 1) ? Color333{7, 0, 0} : Color333{0, 7, 0});
            delay(5);
        }
    }
    delay(2000);

    // Text
    matrix.clear();
    matrix.setCursor(1, 0);
    matrix.setTextSize(1);
    matrix.setTextColor(Color333{7, 7, 7});
    matrix.println("GRID");
}

void setup()
{
    Serial.begin(9600);
    randomSeed(analogRead(0));
    pinMode(0, INPUT_PULLUP);
    smokeTest();
    // app.setup();
}

void loop()
{
    // app.loop(millis());
}
