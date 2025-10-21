#include "App.h"
#include "ArduinoInputProvider.h"
#include "ArduinoLogger.h"
#include "ArduinoPassiveTiming.h"
#include "RGBMatrix32.h"
#include <RGBmatrixPanel.h>
#include "FlashStorage.h"
#include "Input.h"
#include "IStorage.h"
#include "StartScene.h"

// TODO remove
#include "MazeScene.h"
#include "TestScene.h"

// Adafruit flash + FatFs globals
#include "SdFat_Adafruit_Fork.h"
#include <Adafruit_SPIFlash.h>
#include "flash_config.h" // provides flashTransport
static Adafruit_SPIFlash g_flash(&flashTransport);
static FatVolume g_fatfs;

// Most of the signal pins are configurable, but the CLK pin has some
// special constraints.  On 8-bit AVR boards it must be on PORTB...
// Pin 11 works on the Arduino Mega.  On 32-bit SAMD boards it must be
// on the same PORT as the RGB data pins (D2-D7)...
// Pin 8 works on the Adafruit Metro M0 or Arduino Zero,
// Pin A4 works on the Adafruit Metro M4 (if using the Adafruit RGB
// Matrix Shield, cut trace between CLK pads and run a wire to A4).

#define CLK 8 // USE THIS ON ADAFRUIT METRO M0, etc.
// #define CLK A4 // USE THIS ON METRO M4 (not M0)
// #define CLK 11 // USE THIS ON ARDUINO MEGA
#define OE 9
#define LAT 10
#define A A0
#define B A1
#define C A2
#define D A3
#define DB true

// a good target framerate for most scenes
static constexpr double TICK_HZ = 60.0;

static constexpr uint8_t HORIZONTAL_PIN = A5;
static constexpr uint8_t VERTICAL_PIN = A4;
static constexpr uint8_t BUTTON_PIN = 0;

using frames_t = uint16_t; // convenience alias for frame counts

// Globals

static FlashStorage storage(g_flash, g_fatfs);
static RGBmatrixPanel panel(A, B, C, D, CLK, LAT, OE, false);
static RGBMatrix32 gfx{panel};
static ArduinoPassiveTiming timing{TICK_HZ};
static SerialSink sink;
static ArduinoLogger logger(timing, sink);
static InputCalibration calib = ArduinoInputProvider::defaultCalib;
static ArduinoInputProvider inputProvider{HORIZONTAL_PIN, VERTICAL_PIN, BUTTON_PIN, calib};
static Input input{};
static App app{gfx, timing, input, logger, storage};
static unsigned long prev_millis{};
static unsigned long now_millis{};
static millis_t log_last_ms{};
static uint16_t fps_frames{};

void setup()
{
    Serial.begin(115200);
    while (!Serial) // TODO remove for final build
    {
        delay(10);
    }

    randomSeed(analogRead(0));
    pinMode(0, INPUT_PULLUP);
    inputProvider.init();
    input.init(&inputProvider);
    gfx.begin();

    if (!g_flash.begin())
    {
        logger.logf(LogLevel::Warning, "[FlashStorage] flash.begin failed");
        return;
    }
    if (!g_fatfs.begin(&g_flash))
    {
        logger.logf(LogLevel::Warning, "[FlashStorage] fatfs.begin failed");
        return;
    }
    const char *baseDir = "save";
    storage.init(baseDir, &logger);
    calib.load(storage, logger);
    input.setCalibration(calib);

    app.setScene<TestScene>();
    prev_millis = millis();
    log_last_ms = prev_millis;
}

void loop()
{
    now_millis = millis();
    if (now_millis - prev_millis >= static_cast<millis_t>(timing.dtMs()))
    {
        app.loopOnce();
        prev_millis = now_millis;
    }

    // Log every second
    millis_t elapsed = now_millis - log_last_ms;
    if (elapsed >= timing.MILLIS_PER_SEC)
    {
        app.logDiagnostics();
        log_last_ms = now_millis;
    }
}
