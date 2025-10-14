#include "App.h"
#include "ArduinoInputProvider.h"
#include "ArduinoLogger.h"
#include "ArduinoPassiveTiming.h"
#include "BoidsScene.h"
#include "CalibrationScene.h"
#include "ExampleScene.h"
#include "RGBMatrix32.h"
#include <RGBmatrixPanel.h>
#include "FlashStorage.h"
#include "IStorage.h"

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

static RGBmatrixPanel panel(A, B, C, D, CLK, LAT, OE, false);
static RGBMatrix32 gfx{panel};
static ArduinoPassiveTiming timing{TICK_HZ};
static SerialSink sink;
static ArduinoLogger logger(timing, sink);
static ArduinoInputProvider inputProvider{HORIZONTAL_PIN, VERTICAL_PIN, BUTTON_PIN};
static Input input{};
static App app{gfx, timing, input, logger};
static unsigned long prev_millis{};
static unsigned long now_millis{};
static millis_t log_last_ms{};
static uint16_t fps_frames{};

// Simple helper to run once in setup
static void run_flash_storage_smoke(ILogger &logger)
{
    // Mount flash and FatFs (skip if you already do this elsewhere)
    if (!g_flash.begin())
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] flash.begin failed");
        return;
    }
    if (!g_fatfs.begin(&g_flash))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] fatfs.begin failed");
        return;
    }

    FlashStorage storage(g_flash, g_fatfs);
    const char *baseDir = "/save";
    if (!storage.init(baseDir, &logger))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] init failed");
        return;
    }

    const char *fname = "flash_test.bin"; // stored under /save
    const char payload[] = "hello-grid";
    if (!storage.writeAll(fname, payload, sizeof(payload)))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] writeAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FlashStorageTest] write ok");

    char buf[64] = {};
    auto r = storage.readAll(fname, buf, sizeof(buf));
    if (!r)
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] readAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FlashStorageTest] read %d bytes, contents='%s'", r.bytes, buf);

    // Clean up
    if (!storage.removeFile(fname))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] removeFile failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FlashStorageTest] removed ok");
    logger.flush();
}

// Basic smoke test to verify the display is working
static void smokeTest(Matrix32 &gfx, Timing &timing)
{
    gfx.setImmediate(true);

    // Solid green
    gfx.fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, GREEN);
    timing.sleep(2000);

    // Alternating rows
    gfx.clear();
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            gfx.drawPixel(x, y, (y & 1) ? RED : GREEN);
        }
    }
    timing.sleep(2000);

    // Text
    gfx.clear();
    gfx.setCursor(1, 0);
    gfx.setTextSize(1);
    gfx.setTextColor(WHITE);
    gfx.println("GRID");
    timing.sleep(1000);
    gfx.setCursor(1, 10);
    gfx.print("<");
    timing.sleep(1000);
    gfx.advance();
    gfx.print(">");
    timing.sleep(1000);

    gfx.setImmediate(false);
}

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(0));
    pinMode(0, INPUT_PULLUP);
    inputProvider.init();
    input.init(&inputProvider);

    gfx.begin();

    // TODO remove
    while (!Serial)
    {
        delay(10);
    }
    run_flash_storage_smoke(logger);
    // smokeTest(gfx, timing); // uncomment to run smoke tests before main app

    app.setScene<CalibrationScene>();
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
