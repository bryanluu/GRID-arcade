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

void smokeTest()
{
    matrix.begin();

    // Solid green
    matrix.fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, GREEN);
    delay(2000);

    // Alternating rows
    matrix.clear();
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            matrix.drawPixel(x, y, (y & 1) ? RED : GREEN);
            delay(5);
        }
    }
    delay(2000);

    // Text
    matrix.clear();
    matrix.setCursor(1, 0);
    matrix.setTextSize(1);
    matrix.setTextColor(WHITE);
    matrix.println("GRID");
    matrix.setCursor(1, 10);
    matrix.print("<");
    matrix.advance();
    matrix.print(">");
}

void setup()
{
    Serial.begin(9600);
    randomSeed(analogRead(0));
    pinMode(0, INPUT_PULLUP);
    // smokeTest();
    app.setup();
}

void loop()
{
    app.loop(millis());
}
