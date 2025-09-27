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
    matrix.drawPixel(0, 0, rgb(255, 255, 255));
    delay(500);

    // fix the screen with green
    matrix.fillRect(0, 0, 32, 32, rgb(0, 255, 0));
    delay(500);

    // draw a box in yellow
    matrix.drawRect(0, 0, 32, 32, rgb(255, 255, 0));
    delay(500);

    // draw an 'X' in red
    matrix.drawLine(0, 0, 31, 31, rgb(255, 0, 0));
    matrix.drawLine(31, 0, 0, 31, rgb(255, 0, 0));
    delay(500);

    // draw a blue circle
    matrix.drawCircle(10, 10, 10, rgb(0, 0, 255));
    delay(500);

    // fill a violet circle
    matrix.fillCircle(21, 21, 10, rgb(255, 0, 255));
    delay(500);

    matrix.clear();

    // draw some text!
    matrix.setCursor(1, 0);    // start at top left, with one pixel of spacing
    matrix.setTextSize(1);     // size 1 == 8 pixels high
    // matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

    matrix.setTextColor(rgb(255,255,255));
    matrix.println(" Ada");
    matrix.println("fruit");

    // print each letter with a rainbow color
    matrix.setTextColor(rgb(255,0,0));
    matrix.print('3');
    matrix.setTextColor(rgb(255,128,0));
    matrix.print('2');
    matrix.setTextColor(rgb(255,255,0));
    matrix.print('x');
    matrix.setTextColor(rgb(128,255,0));
    matrix.print('3');
    matrix.setTextColor(rgb(0,255,0));
    matrix.println("2");

    matrix.setTextColor(rgb(0,255,255));
    matrix.print('*');
    matrix.setTextColor(rgb(0,128,255));
    matrix.print('R');
    matrix.setTextColor(rgb(0,0,255));
    matrix.print('G');
    matrix.setTextColor(rgb(128,0,255));
    matrix.print('B');
    matrix.setTextColor(rgb(255,0,128));
    matrix.print('*');

    // whew!
}

void ExampleScene::run()
{
  // Do nothing -- image doesn't change
}

ExampleScene exampleScene = ExampleScene();

void setup() 
{
    Serial.begin(9600);
    randomSeed(analogRead(0));
    pinMode(0, INPUT_PULLUP);
    app.setup();
}

void loop() 
{
   app.loop(millis());
}
