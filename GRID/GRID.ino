#include "App.h"
#include "BoidsScene.h"
#include "ExampleScene.h"
#include "helpers.h"
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
#define DB  true

using frames_t = uint16_t; // convenience alias for frame counts

// Globals

static RGBmatrixPanel panel(A, B, C, D, CLK, LAT, OE, false);
static RGBMatrix32 gfx{panel};
static App app{gfx};
static unsigned long prev_millis{};
static unsigned long now_millis{};
static millis_t fps_last_ms{};
static uint16_t fps_frames{};

// Basic smoke test to verify the display is working

static void smokeTest(Matrix32 &gfx)
{
    gfx.setImmediate(true);
    gfx.begin();

    // Solid green
    gfx.fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, GREEN);
    Helpers::sleep(2000);

    // Alternating rows
    gfx.clear();
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            gfx.drawPixel(x, y, (y & 1) ? RED : GREEN);
        }
    }
    Helpers::sleep(2000);

    // Text
    gfx.clear();
    gfx.setCursor(1, 0);
    gfx.setTextSize(1);
    gfx.setTextColor(WHITE);
    gfx.println("GRID");
    Helpers::sleep(1000);
    gfx.setCursor(1, 10);
    gfx.print("<");
    Helpers::sleep(1000);
    gfx.advance();
    gfx.print(">");
    Helpers::sleep(1000);

    gfx.show();
    gfx.setImmediate(false);
}

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(0));
    pinMode(0, INPUT_PULLUP);

    gfx.begin();
    // smokeTest(gfx);
    app.setScene<BoidsScene>();
    prev_millis = millis();
    fps_last_ms = prev_millis;
}

void loop()
{
    now_millis = millis();
    app.loopOnce(now_millis - prev_millis);
    prev_millis = now_millis;

    // FPS counter
	++fps_frames;
	millis_t elapsed = now_millis - fps_last_ms;
	if (elapsed >= 1000) {
		float fps = (1000.0f * fps_frames) / (float)elapsed;
		Serial.print("FPS: ");
		Serial.println(fps, 2); // 2 decimal places

		fps_frames = 0;
		fps_last_ms = now_millis;
	}
}
